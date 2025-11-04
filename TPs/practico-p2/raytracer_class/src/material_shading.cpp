// -----------------------------------------------------------------------------
//  Archivo: material_shading.cpp
//  Descripción: Implementa helpers para calcular iluminación local (Phong) y
//  contribuciones globales por material (reflexión y refracción).
// -----------------------------------------------------------------------------

#include "rt/scene/material_shading.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include "rt/core/random.hpp"
#include "rt/scene/scene.hpp"

namespace
{
    // Epsilon para desplazar rayos secundarios y evitar auto-intersección.
    constexpr double kRayBias = 1e-4;

    // Aproximación de Schlick generalizada para Fresnel.
    double schlick(double cosTheta, double eta_i, double eta_t)
    {
        double r0 = (eta_t - eta_i) / (eta_t + eta_i);
        r0 = r0 * r0;
        double c = std::clamp(cosTheta, 0.0, 1.0);
        return r0 + (1.0 - r0) * std::pow(1.0 - c, 5.0);
    }
} // namespace

namespace shading
{

    // -----------------------------------------------------------------------------
    //  Iluminación local (Phong) con luces puntuales y sombras duras.
    //  Requisitos cubiertos: Consigna Req. 3 (Phong) y Req. 4a (sombras).
    // -----------------------------------------------------------------------------
    Vec3 computeLocalLighting(const Scene &scene, const HitRecord &rec, const Ray &ray)
    {
        Vec3 resultColour(0.0);

        // Ambiente: constante tenue modulada por ka y albedo.
        Vec3 ambient = rec.material.ka * rec.material.albedo;
        resultColour += ambient;

        // Dirección hacia el ojo/cámara.
        Vec3 viewDir = normalized(-ray.direction);

        for (const auto &light : scene.lights)
        {
            // Vector desde el punto hacia la luz y distancia asociada.
            Vec3 lightDir = light.position - rec.point;
            double distanceSquared = length2(lightDir);
            double lightDist = std::sqrt(distanceSquared);
            lightDir /= lightDist; // Normaliza dirección a la luz.

            // Rayo de sombra: si hay obstrucción antes de la luz, se omite.
            Ray shadowRay(rec.point + rec.normal * kRayBias, lightDir);
            HitRecord shadowRec;
            if (scene.intersect(shadowRay, kRayBias, lightDist - kRayBias, shadowRec))
            {
                continue;
            }

            // Difusa (Lambert): max(N·L, 0).
            double ndotl = std::max(dot(rec.normal, lightDir), 0.0);
            Vec3 diffuse = rec.material.kd * rec.material.albedo * ndotl;

            // Especular (Phong): (R·V)^shininess escalado por ks y color especular.
            Vec3 specular(0.0);
            if (rec.material.ks > 0.0 && ndotl > 0.0)
            {
                Vec3 reflectDir = reflect(-lightDir, rec.normal);
                double specAngle = std::max(dot(viewDir, reflectDir), 0.0);
                double specTerm = std::pow(specAngle, rec.material.shininess);
                specular = rec.material.ks * rec.material.specularColour * specTerm;
            }

            // Atenuación cuadrática típica.
            double atten = 1.0 / (1.0 + 0.09 * lightDist + 0.032 * distanceSquared);
            Vec3 lightColour = light.intensity * atten;

            resultColour += (diffuse + specular) * lightColour;
        }

        return resultColour;
    }

    // -----------------------------------------------------------------------------
    //  Contribución global para materiales metálicos (reflexión especular).
    //  Requisito cubierto: Consigna Req. 4b (reflexión).
    //  Nota sobre recursión: `depth` controla cuántos rebotes quedan disponibles.
    //  Cada vez que disparamos un rayo secundario llamamos a `Scene::traceRay`
    //  con `depth - 1`; cuando llega a cero, `traceRay` corta y devuelve negro.
    // -----------------------------------------------------------------------------
    Vec3 computeMetalContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth)
    {
        if (depth <= 1)
        {
            return Vec3(0.0);
        }

        Vec3 unitDir = normalized(ray.direction);
        Vec3 reflectDir = reflect(unitDir, rec.normal);

        if (rec.material.fuzz > 0.0)
        {
            reflectDir = normalized(reflectDir + rec.material.fuzz * rtcore::random_in_unit_sphere());
        }

        Ray reflectRay(rec.point + rec.normal * kRayBias, reflectDir);
        Vec3 reflectColour = scene.traceRay(reflectRay, depth - 1);
        return reflectColour * rec.material.specularColour;
    }

    // -----------------------------------------------------------------------------
    //  Contribución global para materiales dieléctricos (reflexión + refracción).
    //  Requisitos cubiertos: Consigna Req. 4c (refracción) + Fresnel (Schlick) y
    //  absorción opcional (Beer–Lambert) mediante `Material::absorption*`.
    //  También aplica el mismo esquema recursivo: cada rayo reflejado/refractado
    //  consume un nivel de `depth` y, al agotarse, deja de generar color adicional.
    // -----------------------------------------------------------------------------
    Vec3 computeDielectricContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth)
    {
        if (depth <= 1)
        {
            return Vec3(0.0);
        }

        Vec3 unitDir = normalized(ray.direction);

        double eta_i = 1.0;
        double eta_t = rec.material.refractiveIndex;
        Vec3 normal = rec.normal;
        if (!rec.frontFace)
        {
            std::swap(eta_i, eta_t);
            normal = -normal;
        }

        double cosTheta = std::min(dot(-unitDir, normal), 1.0);
        double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));

        bool cannotRefract = (eta_i / eta_t) * sinTheta > 1.0;
        Vec3 refractedDir(0.0);
        double reflectance = 1.0;

        if (!cannotRefract && refract(unitDir, normal, eta_i, eta_t, refractedDir))
        {
            reflectance = schlick(cosTheta, eta_i, eta_t);
        }

        Vec3 reflectDir = reflect(unitDir, normal);
        Vec3 reflectColour = scene.traceRay(Ray(rec.point + normal * kRayBias, reflectDir), depth - 1) * rec.material.specularColour;

        Vec3 refractColour(0.0);
        if (reflectance < 1.0 && !cannotRefract)
        {
            Vec3 refractedNorm = normalized(refractedDir);
            Vec3 refractOrigin = rec.point - normal * kRayBias;
            refractColour = scene.traceRay(Ray(refractOrigin, refractedNorm), depth - 1);

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

        return reflectColour * reflectance + refractColour * (1.0 - reflectance);
    }

    // -----------------------------------------------------------------------------
    //  Selección de contribución global según `MaterialType`. Si el material es
    //  difuso, no se disparan rayos secundarios y `depth` queda sin cambios para
    //  el resto de la rama de trazado.
    // -----------------------------------------------------------------------------
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
            return Vec3(0.0);
        }
    }

} // namespace shading
