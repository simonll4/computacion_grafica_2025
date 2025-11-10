// -----------------------------------------------------------------------------
//  Contenedor de escena y trazado de rayos
// -----------------------------------------------------------------------------
//  Agrupa todos los elementos de la escena:
//   - objects: geometría (esferas, planos, etc.)
//   - lights: fuentes de luz puntuales
//   - backgroundColour: color del cielo cuando no hay intersección
//   - maxDepth: límite de rebotes recursivos (reflexión/refracción)
//
//  Funciones principales:
//   - intersect(): encuentra la intersección más cercana del rayo
//   - traceRay(): calcula el color final (implementado en scene.cpp)
// -----------------------------------------------------------------------------

#pragma once

#include <vector>
#include <memory>
#include <limits>

#include "rt/scene/object.hpp"
#include "rt/scene/light.hpp"

class Scene
{
public:
    std::vector<ObjectPtr> objects; // Geometría de la escena
    std::vector<Light> lights;      // Luces puntuales
    Vec3 backgroundColour;          // Color de fondo (cielo)
    int maxDepth;                   // Profundidad máxima de rebotes

    Scene() : backgroundColour(0.7, 0.8, 1.0), maxDepth(5) {}

    void addObject(const ObjectPtr &obj) { objects.push_back(obj); }
    void addLight(const Light &light) { lights.push_back(light); }

    // Encuentra la intersección más cercana en el rango [t_min, t_max]
    // Retorna true si hubo impacto y llena 'rec' con los datos
    bool intersect(const Ray &ray, double t_min, double t_max, HitRecord &rec) const
    {
        HitRecord tempRec;
        bool hitAnything = false;
        double closestSoFar = t_max;

        for (const auto &obj : objects)
        {
            if (obj->intersect(ray, t_min, closestSoFar, tempRec))
            {
                hitAnything = true;
                closestSoFar = tempRec.t;
                rec = tempRec;
            }
        }

        return hitAnything;
    }

    // Traza un rayo y calcula su color final (implementado en scene.cpp)
    Vec3 traceRay(const Ray &ray, int depth) const;
};
