// -----------------------------------------------------------------------------
//  Archivo: scene.hpp
//  Descripción: Contenedor de objetos y luces, y API de intersección y trazado.
//  `intersect` encuentra el hit más cercano dentro de [t_min, t_max] y
//  `traceRay` implementa el shading: fondo, modelo local (Phong con sombras) y
//  contribuciones globales (reflexión/refracción) con control por `maxDepth`.
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
    std::vector<ObjectPtr> objects;
    std::vector<Light> lights;
    Vec3 backgroundColour;
    int maxDepth;

    Scene() : backgroundColour(0.7, 0.8, 1.0), maxDepth(5) {}

    void addObject(const ObjectPtr &obj) { objects.push_back(obj); }
    void addLight(const Light &light) { lights.push_back(light); }

    // Busca la intersección más cercana del rayo en el rango [t_min, t_max].
    // Si encuentra, escribe en `rec` y devuelve true.
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

    // Trazado de rayos con profundidad límite (definido en `maxDepth`).
    Vec3 traceRay(const Ray &ray, int depth) const;
};
