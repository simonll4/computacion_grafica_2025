// -----------------------------------------------------------------------------
//  API de iluminación local y global
// -----------------------------------------------------------------------------
//  Funciones para calcular shading según tipo de material:
//   - computeLocalLighting: modelo Phong + sombras (difuso + especular).
//   - computeMetalContribution: reflexión especular con rugosidad.
//   - computeDielectricContribution: refracción + reflexión (Fresnel) en dieléctricos.
//   - computeGlobalContribution: dispatcher según MaterialType.
// -----------------------------------------------------------------------------

#pragma once

#include "rt/core/vec3.hpp"
#include "rt/core/ray.hpp"
#include "rt/scene/material.hpp"
#include "rt/scene/object.hpp"

class Scene;

namespace shading
{

Vec3 computeLocalLighting(const Scene &scene, const HitRecord &rec, const Ray &ray);
Vec3 computeMetalContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth);
Vec3 computeDielectricContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth);
Vec3 computeGlobalContribution(const Scene &scene, const HitRecord &rec, const Ray &ray, int depth);

} // namespace shading
