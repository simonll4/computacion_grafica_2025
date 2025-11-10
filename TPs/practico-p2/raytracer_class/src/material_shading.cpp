// -----------------------------------------------------------------------------
//  Modelos de iluminación local y global
// -----------------------------------------------------------------------------
//  Implementa tres sistemas de shading:
//   1. Local (Phong): ambiente + difuso + especular, con test de sombras.
//   2. Metal: reflexión especular con rugosidad (fuzz).
//   3. Dieléctrico: refracción + reflexión según ángulo (Fresnel).
//
//  Modelo Phong (local):
//   I_local = I_a + Σ_luces {[ I_d · max(N·L,0) + I_s · (R·V)^n ] · atten}
//   donde: I_a = ka·albedo, I_d = kd·albedo, I_s = ks·specularColour, atten = 1/(1+k1·d+k2·d²)
//
//  Contribuciones globales:
//   - Difuso: no aporta (solo local).
//   - Metal: reflexión R = I - 2(I·N)N, con perturbación opcional.
//   - Dieléctrico: mezcla reflexión/refracción con Fresnel-Schlick.
// -----------------------------------------------------------------------------

#include "rt/scene/material_shading.hpp"

#include <algorithm>
#include <cmath>

#include "rt/core/random.hpp"
#include "rt/scene/scene.hpp"

namespace
{
    // Bias para evitar auto-intersecciones por error numérico
    constexpr double kRayBias = 1e-4;
} // namespace

namespace shading
{
    // -------------------------------------------------------------------------
    //  Iluminación local Phong con sombras
    // -------------------------------------------------------------------------
    //  Calcula: I_ambient + Σ_luces [I_diffuse + I_specular] · attenuation.
    //  Si hay oclusión entre el punto y la luz, esa luz no contribuye.
    // -------------------------------------------------------------------------
    Vec3 computeLocalLighting(const Scene &scene, const HitRecord &rec, const Ray &ray)
    {
        Vec3 resultColour(0.0);

        // Componente AMBIENTE: luz constante (aproxima GI con ka)
        Vec3 ambient = rec.material.ka * rec.material.albedo;
        resultColour += ambient;

        Vec3 viewDir = normalized(-ray.direction); // Hacia el observador

        for (const auto &light : scene.lights)
        {
            // Dirección y distancia a la luz
            Vec3 lightDir = light.position - rec.point;
            double distanceSquared = length2(lightDir);
            double lightDist = std::sqrt(distanceSquared);
            lightDir /= lightDist; // Normaliza

            // Test de sombras: rayo hacia la luz
            Ray shadowRay(rec.point + rec.normal * kRayBias, lightDir);
            HitRecord shadowRec;
            if (scene.intersect(shadowRay, kRayBias, lightDist - kRayBias, shadowRec))
            {
                continue; // Punto en sombra
            }

            // Componente DIFUSA: Lambert (max(N·L, 0))
            double ndotl = std::max(dot(rec.normal, lightDir), 0.0);
            Vec3 diffuse = rec.material.kd * rec.material.albedo * ndotl;

            // Componente ESPECULAR: brillo especular (Phong) (R·V)^shininess
            Vec3 specular(0.0);
            if (rec.material.ks > 0.0 && ndotl > 0.0)
            {
                Vec3 reflectDir = reflect(-lightDir, rec.normal);
                double specAngle = std::max(dot(viewDir, reflectDir), 0.0);
                double specTerm = std::pow(specAngle, rec.material.shininess);
                specular = rec.material.ks * rec.material.specularColour * specTerm;
            }

            // Atenuación por distancia: 1 / (1 + k₁·d + k₂·d²)
            double atten = 1.0 / (1.0 + 0.09 * lightDist + 0.032 * distanceSquared);
            Vec3 lightColour = light.intensity * atten;

            resultColour += (diffuse + specular) * lightColour;
        }

        return resultColour;
    }

    // -------------------------------------------------------------------------
    //  Reflexión metálica con rugosidad
    // -------------------------------------------------------------------------
    //  Implementa: R = I - 2(I·N)N + fuzz·rand.
    //  fuzz=0: espejo perfecto. fuzz>0: metal cepillado.
    // -------------------------------------------------------------------------
    Vec3 computeMetalContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth)
    {
        if (depth <= 1)
        {
            return Vec3(0.0);
        }

        // Reflexión perfecta: R = I - 2(I·N)N
        Vec3 unitDir = normalized(ray.direction);
        Vec3 reflectDir = reflect(unitDir, rec.normal);

        // Rugosidad: perturba con vector aleatorio
        if (rec.material.fuzz > 0.0)
        {
            reflectDir = normalized(reflectDir + rec.material.fuzz * rtcore::random_in_unit_sphere());
        }

        Ray reflectRay(rec.point + rec.normal * kRayBias, reflectDir);
        Vec3 reflectColour = scene.traceRay(reflectRay, depth - 1);

        return reflectColour * rec.material.specularColour; // Tinte del metal
    }

    // -------------------------------------------------------------------------
    //  Refracción dieléctrica con Fresnel
    // -------------------------------------------------------------------------
    //  Implementa:
    //   - Ley de Snell: η₁·sinθ₁ = η₂·sinθ₂.
    //   - Fresnel-Schlick: R(θ) = R₀ + (1-R₀)(1-cosθ)⁵.
    //   - Reflexión interna total si η₁·sinθ₁ > η₂.
    //  Mezcla reflexión/refracción de forma determinística según Fresnel.
    // -------------------------------------------------------------------------
    Vec3 computeDielectricContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth)
    {
        if (depth <= 1)
        {
            return Vec3(0.0);
        }

        Vec3 unitDir = normalized(ray.direction);

        // Índices de refracción según sentido (entrada/salida)
        double eta_i = rec.frontFace ? 1.0 : rec.material.refractiveIndex;
        double eta_t = rec.frontFace ? rec.material.refractiveIndex : 1.0;
        double eta_ratio = eta_i / eta_t;

        // Ángulos para Snell y Fresnel
        double cosTheta = std::min(dot(-unitDir, rec.normal), 1.0);
        double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);

        // Reflexión interna total si η·sinθ > 1
        bool cannotRefract = eta_ratio * sinTheta > 1.0;

        // Fresnel-Schlick: R(θ) = R₀ + (1-R₀)(1-cosθ)⁵
        double r0 = (eta_t - eta_i) / (eta_t + eta_i);
        r0 = r0 * r0;
        double reflectance = r0 + (1.0 - r0) * std::pow(1.0 - cosTheta, 5.0);

        if (cannotRefract)
        {
            // Solo reflexión (100%)
            Vec3 reflectDir = reflect(unitDir, rec.normal);
            Ray reflectRay(rec.point + rec.normal * kRayBias, reflectDir);
            return scene.traceRay(reflectRay, depth - 1) * rec.material.albedo;
        }

        // Mezcla reflexión y refracción según Fresnel
        Vec3 reflectDir = reflect(unitDir, rec.normal);
        Ray reflectRay(rec.point + rec.normal * kRayBias, reflectDir);
        Vec3 reflectColour = scene.traceRay(reflectRay, depth - 1);

        Vec3 refractDir;
        if (!refract(unitDir, rec.normal, eta_i, eta_t, refractDir))
        {
            return reflectColour * rec.material.albedo; // Respaldo si falla refracción
        }

        Vec3 refrDirNorm = normalized(refractDir);
        Ray refractRay(rec.point + refrDirNorm * kRayBias, refrDirNorm);
        Vec3 refractColour = scene.traceRay(refractRay, depth - 1);

        // Atenúa el color transmitido usando Beer-Lambert si el material define absorción
        const Vec3 &sigma = rec.material.absorption;
        if (sigma.x > 0.0 || sigma.y > 0.0 || sigma.z > 0.0)
        {
            double path = rec.material.absorptionDistance;
            if (path <= 0.0)
            {
                path = 1.0; // Distancia mínima para evitar atenuación nula
            }

            Vec3 transmittance(
                std::exp(-sigma.x * path),
                std::exp(-sigma.y * path),
                std::exp(-sigma.z * path));
            refractColour *= transmittance;
        }

        // Mezcla determinística: R·reflectance + T·(1-reflectance)
        Vec3 finalColour = reflectColour * reflectance + refractColour * (1.0 - reflectance);

        return finalColour * rec.material.albedo; // Tinte base del material (absorción aplicada arriba)
    }

    // -------------------------------------------------------------------------
    //  Dispatcher de contribución global
    // -------------------------------------------------------------------------
    Vec3 computeGlobalContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth)
    {
        switch (rec.material.type)
        {
        case MaterialType::Metal:
            return computeMetalContribution(scene, rec, ray, depth);

        case MaterialType::Dielectric:
            return computeDielectricContribution(scene, rec, ray, depth);

        case MaterialType::Diffuse:
        default:
            return Vec3(0.0); // Difusos no aportan globalmente
        }
    }

} // namespace shading
