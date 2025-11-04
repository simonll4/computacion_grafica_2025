// -----------------------------------------------------------------------------
//  Archivo: material_shading.cpp
//  Descripción: Implementación de modelos de iluminación y shading por material.
//  Contiene tres componentes principales:
//   1. Iluminación local (Phong): ambiente + difuso + especular con sombras
//   2. Reflexión metálica: con rugosidad opcional (fuzz)
//   3. Refracción dieléctrica: con Fresnel, reflexión interna y absorción
//
//  Modelo Phong:
//   - Ambiente: ka * albedo (luz constante de fondo)
//   - Difuso: kd * albedo * max(N·L, 0) (Lambert)
//   - Especular: ks * specColour * (R·V)^shininess (highlight brillante)
//
//  Materiales:
//   - Diffuse: solo iluminación local (no reflexión/refracción)
//   - Metal: reflexión especular con rugosidad controlada por 'fuzz'
//   - Dielectric: mezcla reflexión/refracción según ángulo (Fresnel-Schlick)
// -----------------------------------------------------------------------------

#include "rt/scene/material_shading.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include "rt/core/random.hpp"
#include "rt/scene/scene.hpp"

namespace
{
    // Desplazamiento mínimo para evitar auto-intersecciones por error numérico.
    // Los rayos secundarios (sombra, reflexión, refracción) se originan ligeramente
    // desplazados de la superficie para no intersectar el mismo objeto.
    constexpr double kRayBias = 1e-4;

    /// Aproximación de Fresnel-Schlick: calcula reflectancia en interfaz dieléctrica.
    /// @param cosTheta Coseno del ángulo entre rayo incidente y normal
    /// @param eta_i Índice de refracción del medio de entrada
    /// @param eta_t Índice de refracción del medio de salida
    /// @return Fracción de luz reflejada [0,1] (el resto se refracta)
    double schlick(double cosTheta, double eta_i, double eta_t)
    {
        // R₀ = reflectancia en incidencia normal (ángulo 0°)
        double r0 = (eta_t - eta_i) / (eta_t + eta_i);
        r0 = r0 * r0;
        
        // Clampea el coseno al rango válido
        double c = std::clamp(cosTheta, 0.0, 1.0);
        
        // Fórmula de Schlick: R(θ) = R₀ + (1-R₀)(1-cos θ)⁵
        // A mayor ángulo (menor cos), más reflexión (efecto de ángulo rasante)
        return r0 + (1.0 - r0) * std::pow(1.0 - c, 5.0);
    }
} // namespace

namespace shading
{
    /// Calcula iluminación local usando el modelo de Phong con sombras.
    /// Itera sobre todas las luces y acumula contribuciones ambiente, difusa y especular.
    /// @param scene Escena con objetos y luces
    /// @param rec Información del punto de intersección
    /// @param ray Rayo incidente (para calcular dirección de vista)
    /// @return Color resultante de iluminación directa
    Vec3 computeLocalLighting(const Scene &scene, const HitRecord &rec, const Ray &ray)
    {
        Vec3 resultColour(0.0);

        // ═══════════════════════════════════════════════════════════════════
        // Componente AMBIENTE: luz constante que ilumina todo uniformemente
        // ═══════════════════════════════════════════════════════════════════
        // Simula luz indirecta rebotada múltiples veces en la escena.
        // ka controla la intensidad, albedo el color del material.
        Vec3 ambient = rec.material.ka * rec.material.albedo;
        resultColour += ambient;

        // Dirección hacia el observador (cámara), necesaria para especular
        Vec3 viewDir = normalized(-ray.direction);

        // Itera sobre cada luz puntual de la escena
        for (const auto &light : scene.lights)
        {
            // ═══════════════════════════════════════════════════════════════
            // Cálculo de dirección y distancia a la luz
            // ═══════════════════════════════════════════════════════════════
            Vec3 lightDir = light.position - rec.point;
            double distanceSquared = length2(lightDir);
            double lightDist = std::sqrt(distanceSquared);
            lightDir /= lightDist; // Normaliza para obtener dirección unitaria

            // ═══════════════════════════════════════════════════════════════
            // Test de SOMBRAS: lanza rayo desde el punto hacia la luz
            // ═══════════════════════════════════════════════════════════════
            // Si hay obstrucción entre el punto y la luz, esta luz no contribuye.
            // El bias evita que el punto se sombree a sí mismo por error numérico.
            Ray shadowRay(rec.point + rec.normal * kRayBias, lightDir);
            HitRecord shadowRec;
            if (scene.intersect(shadowRay, kRayBias, lightDist - kRayBias, shadowRec))
            {
                continue; // Punto en sombra respecto a esta luz
            }

            // ═══════════════════════════════════════════════════════════════
            // Componente DIFUSA (Lambert): refleja luz en todas direcciones
            // ═══════════════════════════════════════════════════════════════
            // Intensidad proporcional al coseno del ángulo (N·L).
            // Superficies perpendiculares a la luz reciben más energía.
            double ndotl = std::max(dot(rec.normal, lightDir), 0.0);
            Vec3 diffuse = rec.material.kd * rec.material.albedo * ndotl;

            // ═══════════════════════════════════════════════════════════════
            // Componente ESPECULAR (Phong): highlight brillante
            // ═══════════════════════════════════════════════════════════════
            // Solo visible cuando la luz se refleja hacia el observador.
            // shininess controla el tamaño del highlight (mayor = más pequeño).
            Vec3 specular(0.0);
            if (rec.material.ks > 0.0 && ndotl > 0.0)
            {
                // Dirección de reflexión perfecta de la luz
                Vec3 reflectDir = reflect(-lightDir, rec.normal);
                
                // Ángulo entre reflexión y vista: determina intensidad especular
                double specAngle = std::max(dot(viewDir, reflectDir), 0.0);
                
                // Exponente de Phong: concentra el highlight
                double specTerm = std::pow(specAngle, rec.material.shininess);
                
                specular = rec.material.ks * rec.material.specularColour * specTerm;
            }

            // ═══════════════════════════════════════════════════════════════
            // ATENUACIÓN: la luz decrece con la distancia
            // ═══════════════════════════════════════════════════════════════
            // Modelo cuadrático: 1 / (1 + k₁·d + k₂·d²)
            // Simula cómo la luz se dispersa en el espacio 3D.
            double atten = 1.0 / (1.0 + 0.09 * lightDist + 0.032 * distanceSquared);
            Vec3 lightColour = light.intensity * atten;

            // Acumula contribución de esta luz (difusa + especular) * intensidad
            resultColour += (diffuse + specular) * lightColour;
        }

        return resultColour;
    }

    /// Calcula contribución global para materiales metálicos (reflexión especular).
    /// Los metales reflejan la luz como un espejo, con rugosidad opcional (fuzz).
    /// @param scene Escena para trazar rayos reflejados
    /// @param rec Información del punto de intersección
    /// @param ray Rayo incidente
    /// @param depth Profundidad recursiva restante
    /// @return Color reflejado modulado por el color especular del metal
    Vec3 computeMetalContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth)
    {
        // Caso base: no más rebotes permitidos
        if (depth <= 1)
        {
            return Vec3(0.0);
        }

        // Calcula dirección de reflexión perfecta: R = I - 2(I·N)N
        Vec3 unitDir = normalized(ray.direction);
        Vec3 reflectDir = reflect(unitDir, rec.normal);

        // RUGOSIDAD (fuzz): perturba la reflexión para simular microfacetas
        // fuzz=0: espejo perfecto, fuzz>0: reflexión difusa (metal cepillado)
        if (rec.material.fuzz > 0.0)
        {
            // Agrega vector aleatorio escalado por fuzz
            reflectDir = normalized(reflectDir + rec.material.fuzz * rtcore::random_in_unit_sphere());
        }

        // Lanza rayo reflejado (con bias para evitar auto-intersección)
        Ray reflectRay(rec.point + rec.normal * kRayBias, reflectDir);
        
        // Traza recursivamente y modula por el color especular del metal
        // Esto permite metales coloreados (oro, cobre, etc.)
        Vec3 reflectColour = scene.traceRay(reflectRay, depth - 1);
        return reflectColour * rec.material.specularColour;
    }

    /// Calcula contribución global para materiales dieléctricos (vidrio, agua).
    /// Implementa refracción con ley de Snell, reflexión interna total, Fresnel
    /// y absorción volumétrica (Beer-Lambert) para vidrios coloreados.
    /// @param scene Escena para trazar rayos reflejados/refractados
    /// @param rec Información del punto de intersección
    /// @param ray Rayo incidente
    /// @param depth Profundidad recursiva restante
    /// @return Mezcla de reflexión y refracción según ángulo (Fresnel)
    Vec3 computeDielectricContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth)
    {
        // Caso base: no más rebotes permitidos
        if (depth <= 1)
        {
            return Vec3(0.0);
        }

        Vec3 unitDir = normalized(ray.direction);

        // ═══════════════════════════════════════════════════════════════════
        // Determina índices de refracción según si entramos o salimos
        // ═══════════════════════════════════════════════════════════════════
        double eta_i = 1.0;  // Índice del medio de entrada (aire por defecto)
        double eta_t = rec.material.refractiveIndex; // Índice del material
        Vec3 normal = rec.normal;
        
        if (!rec.frontFace)
        {
            // Saliendo del material: invertimos índices y normal
            std::swap(eta_i, eta_t);
            normal = -normal;
        }

        // ═══════════════════════════════════════════════════════════════════
        // Calcula ángulos y verifica reflexión interna total
        // ═══════════════════════════════════════════════════════════════════
        // Ley de Snell: η₁ sin θ₁ = η₂ sin θ₂
        // Si η₁ sin θ₁ > η₂, no hay refracción (reflexión total interna)
        double cosTheta = std::min(dot(-unitDir, normal), 1.0);
        double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));

        // Verifica si hay reflexión total interna
        bool cannotRefract = (eta_i / eta_t) * sinTheta > 1.0;
        Vec3 refractedDir(0.0);
        double reflectance = 1.0; // Por defecto, 100% reflexión

        // ═══════════════════════════════════════════════════════════════════
        // Calcula dirección refractada y coeficiente de Fresnel
        // ═══════════════════════════════════════════════════════════════════
        if (!cannotRefract && refract(unitDir, normal, eta_i, eta_t, refractedDir))
        {
            // Fresnel-Schlick: determina qué fracción se refleja vs. refracta
            // A ángulos rasantes, más reflexión (por eso el vidrio refleja en bordes)
            reflectance = schlick(cosTheta, eta_i, eta_t);
        }

        // ═══════════════════════════════════════════════════════════════════
        // Componente REFLEJADA: siempre presente en dieléctricos
        // ═══════════════════════════════════════════════════════════════════
        Vec3 reflectDir = reflect(unitDir, normal);
        Vec3 reflectColour = scene.traceRay(Ray(rec.point + normal * kRayBias, reflectDir), depth - 1) 
                           * rec.material.specularColour;

        // ═══════════════════════════════════════════════════════════════════
        // Componente REFRACTADA: solo si no hay reflexión total
        // ═══════════════════════════════════════════════════════════════════
        Vec3 refractColour(0.0);
        if (reflectance < 1.0 && !cannotRefract)
        {
            Vec3 refractedNorm = normalized(refractedDir);
            
            // Origen del rayo refractado: dentro del material (bias negativo)
            Vec3 refractOrigin = rec.point - normal * kRayBias;
            refractColour = scene.traceRay(Ray(refractOrigin, refractedNorm), depth - 1);

            // ═══════════════════════════════════════════════════════════════
            // ABSORCIÓN volumétrica (Beer-Lambert): vidrios coloreados
            // ═══════════════════════════════════════════════════════════════
            // La luz se atenúa exponencialmente al atravesar el material.
            // σ = coeficiente de absorción, d = distancia de referencia
            // Transmitancia = e^(-σ·d) por canal RGB
            if (length2(rec.material.absorption) > 0.0 && rec.material.absorptionDistance > 0.0)
            {
                Vec3 sigma = rec.material.absorption;
                double d = rec.material.absorptionDistance;
                Vec3 transmittance(std::exp(-sigma.x * d),
                                   std::exp(-sigma.y * d),
                                   std::exp(-sigma.z * d));
                refractColour *= transmittance;
            }
        }

        // Mezcla reflexión y refracción según Fresnel
        // reflectance ∈ [0,1]: fracción reflejada
        return reflectColour * reflectance + refractColour * (1.0 - reflectance);
    }

    /// Dispatcher: selecciona la función de contribución global según el tipo de material.
    /// @param scene Escena para trazado recursivo
    /// @param rec Información del punto de intersección con material
    /// @param ray Rayo incidente
    /// @param depth Profundidad recursiva restante
    /// @return Contribución global (reflexión/refracción) o negro si es difuso
    Vec3 computeGlobalContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth)
    {
        // Despacha a la función apropiada según el tipo de material
        switch (rec.material.type)
        {
        case MaterialType::Metal:
            // Metales: reflexión especular con rugosidad opcional
            return computeMetalContribution(scene, rec, ray, depth);
            
        case MaterialType::Dielectric:
            // Dieléctricos: mezcla reflexión/refracción con Fresnel
            return computeDielectricContribution(scene, rec, ray, depth);
            
        case MaterialType::Diffuse:
        default:
            // Difusos: no contribuyen globalmente (solo iluminación local)
            return Vec3(0.0);
        }
    }

} // namespace shading
