// -----------------------------------------------------------------------------
//  Archivo: scene.cpp
//  Descripción general:
//  - Consigna Requisito 3 (modelo local): ambiente + difusa + especular (Phong)
//  - Consigna Requisito 4: sombras (4a), reflexión (4b) y refracción (4c)
//  - Consigna Requisito 5: control de profundidad (maxDepth) y color de fondo
//
//  La lógica específica de cada material vive en
//  `rt/scene/material_shading.hpp`; este archivo queda centrado en el flujo
//  general del trazado: color de fondo, iluminación local y contribución global.
//

#include <algorithm>
#include <cmath>
#include <limits>

#include "rt/scene/scene.hpp"
#include "rt/scene/material_shading.hpp"
#include "rt/core/vec3.hpp"

// Calcula el color obtenido al lanzar un rayo en la escena. Flujo general:
// 1) Si se agotó la profundidad, cortar (negro)
// 2) Si no hay intersección, devolver color de fondo (gradiente)
// 3) En el primer impacto: sumar iluminación local (ambiente + difusa + especular)
// 4) Según el material, sumar contribución global (reflexión y/o refracción)
// 5) Devolver suma de local + global
// Nota: los rayos secundarios usan un pequeño desplazamiento (1e-4) para
// evitar auto-intersecciones por errores numéricos (acné).
Vec3 Scene::traceRay(const Ray &ray, int depth) const
{
    if (depth <= 0) // Consigna - Req. 5: límite de rebotes recursivos
    {
        // Se alcanzó el máximo permitido de rebotes: devuelve negro.
        return Vec3(0, 0, 0);
    }

    HitRecord rec{};
    if (!intersect(ray, 1e-4, std::numeric_limits<double>::infinity(), rec))
    {
        // Consigna - Req. 5: color de fondo.
        // Sin intersección: usa gradiente vertical (blanco→backgroundColour).
        Vec3 unitDir = normalized(ray.direction);
        double t = 0.5 * (unitDir.y + 1.0);
        // Mezcla lineal entre blanco y el color configurado en la escena.
        return (1.0 - t) * Vec3(1.0, 1.0, 1.0) + t * backgroundColour;
    }

    // Iluminación local (Phong) + contribución global según el material.
    Vec3 localColour = shading::computeLocalLighting(*this, rec, ray);
    Vec3 globalColour = shading::computeGlobalContribution(*this, rec, ray, depth);
    return localColour + globalColour;
}
