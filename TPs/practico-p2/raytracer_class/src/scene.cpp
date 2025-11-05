// -----------------------------------------------------------------------------
// Trazado recursivo de rayos (corazón del raytracer)
// -----------------------------------------------------------------------------
//  Implementa Scene::traceRay(), que determina el color de un rayo mediante:
//   1. Test de intersección con geometría
//   2. Iluminación local (Phong) para superficies opacas
//   3. Reflexión/refracción recursiva según el material
//
//  Flujo:
//   - Si depth <= 0: retorna negro (límite de recursión alcanzado)
//   - Si no hay hit: retorna gradiente de fondo (cielo)
//   - Si hay hit: calcula iluminación según tipo de material
//      * Dieléctrico: solo contribución global (transparente)
//      * Difuso/Metal: local + global
// -----------------------------------------------------------------------------

#include <algorithm>
#include <cmath>
#include <limits>

#include "rt/scene/scene.hpp"
#include "rt/scene/material_shading.hpp"
#include "rt/scene/material.hpp"
#include "rt/core/vec3.hpp"

Vec3 Scene::traceRay(const Ray &ray, int depth) const
{
    // Caso base: profundidad máxima alcanzada
    if (depth <= 0)
    {
        return Vec3(0, 0, 0);
    }

    // Busca intersección más cercana (bias 1e-4 evita auto-intersecciones)
    HitRecord rec{};
    if (!intersect(ray, 1e-4, std::numeric_limits<double>::infinity(), rec))
    {
        // Sin intersección: genera gradiente cielo
        Vec3 unitDir = normalized(ray.direction);
        double t = 0.5 * (unitDir.y + 1.0); // Mapea Y de [-1,1] a [0,1]
        return (1.0 - t) * Vec3(1.0, 1.0, 1.0) + t * backgroundColour;
    }

    // Dieléctricos: solo contribución global (sin iluminación local)
    if (rec.material.type == MaterialType::Dielectric)
    {
        return shading::computeGlobalContribution(*this, rec, ray, depth);
    }

    // Otros materiales: local (Phong + sombras) + global (reflexiones)
    Vec3 localColour = shading::computeLocalLighting(*this, rec, ray);
    Vec3 globalColour = shading::computeGlobalContribution(*this, rec, ray, depth);

    return localColour + globalColour;
}
