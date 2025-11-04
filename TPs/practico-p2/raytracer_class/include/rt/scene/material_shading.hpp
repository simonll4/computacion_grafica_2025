// -----------------------------------------------------------------------------
//  Archivo: material_shading.hpp
//  Descripción: Funciones auxiliares para calcular iluminación local (Phong)
//  y contribuciones globales dependientes del material (reflexión, refracción).
//  Encapsula la lógica específica de cada `MaterialType` para mantener
//  `Scene::traceRay` más legible.
// -----------------------------------------------------------------------------

#pragma once

#include "rt/core/vec3.hpp"
#include "rt/core/ray.hpp"
#include "rt/scene/material.hpp"
#include "rt/scene/object.hpp"

class Scene;

namespace shading
{

// Iluminación local (Phong) considerando luces puntuales y sombras.
Vec3 computeLocalLighting(const Scene &scene, const HitRecord &rec, const Ray &ray);

// Contribución recursiva para materiales metálicos (reflexión especular).
Vec3 computeMetalContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth);

// Contribución recursiva para materiales dieléctricos (reflexión + refracción).
Vec3 computeDielectricContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth);

// Selecciona la contribución global según `rec.material.type`.
Vec3 computeGlobalContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth);

} // namespace shading

