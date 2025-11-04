// -----------------------------------------------------------------------------
//  Archivo: scene.cpp
//  Descripción: Implementación del trazado de rayos recursivo (Scene::traceRay).
//  Este es el corazón del raytracer: determina el color final de un rayo
//  combinando iluminación local (Phong) y contribuciones globales (reflexión/
//  refracción) según el material del objeto intersectado.
//
//  Algoritmo:
//   1. Verifica profundidad recursiva (evita stack overflow)
//   2. Busca intersección más cercana con objetos de la escena
//   3. Si no hay hit: retorna color de fondo (gradiente cielo)
//   4. Si hay hit: calcula iluminación local + contribución global recursiva
// -----------------------------------------------------------------------------

#include <algorithm>
#include <cmath>
#include <limits>

#include "rt/scene/scene.hpp"
#include "rt/scene/material_shading.hpp"
#include "rt/core/vec3.hpp"

/// Traza un rayo en la escena y retorna el color resultante.
/// @param ray Rayo a trazar (primario o secundario)
/// @param depth Profundidad recursiva restante (evita bucles infinitos)
/// @return Color RGB acumulado por este rayo
Vec3 Scene::traceRay(const Ray &ray, int depth) const
{
    // Caso base: si alcanzamos la profundidad máxima, no aportamos más luz
    // Esto previene recursión infinita en escenas con múltiples reflexiones
    if (depth <= 0)
    {
        return Vec3(0, 0, 0); // Negro (sin contribución)
    }

    // Busca la intersección más cercana del rayo con todos los objetos
    // El rango [1e-4, ∞) evita auto-intersecciones por error numérico
    HitRecord rec{};
    if (!intersect(ray, 1e-4, std::numeric_limits<double>::infinity(), rec))
    {
        // No hay intersección: retorna color de fondo (gradiente cielo)
        // Interpola linealmente entre blanco (abajo) y backgroundColour (arriba)
        // basándose en la componente Y de la dirección normalizada del rayo
        Vec3 unitDir = normalized(ray.direction);
        double t = 0.5 * (unitDir.y + 1.0); // Mapea [-1,1] a [0,1]
        return (1.0 - t) * Vec3(1.0, 1.0, 1.0) + t * backgroundColour;
    }

    // Hay intersección: calcula iluminación en el punto de impacto
    
    // Componente LOCAL: iluminación directa de las luces (modelo Phong)
    // Incluye: ambiente + difuso + especular, con test de sombras
    Vec3 localColour = shading::computeLocalLighting(*this, rec, ray);
    
    // Componente GLOBAL: reflexiones/refracciones recursivas según material
    // - Difuso: no aporta (retorna negro)
    // - Metal: reflexión especular con posible rugosidad (fuzz)
    // - Dieléctrico: mezcla reflexión y refracción según Fresnel
    Vec3 globalColour = shading::computeGlobalContribution(*this, rec, ray, depth);
    
    // Color final = iluminación local + contribución global
    return localColour + globalColour;
}
