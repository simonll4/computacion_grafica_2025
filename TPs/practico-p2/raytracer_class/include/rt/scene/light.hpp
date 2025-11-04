// -----------------------------------------------------------------------------
//  Archivo: light.hpp
//  Consigna - Requisito 3: luz puntual utilizada por el modelo de iluminación
//  local (Phong) en `Scene::traceRay`.
// -----------------------------------------------------------------------------

#pragma once

#include "rt/core/vec3.hpp"

struct Light
{
    Vec3 position;
    Vec3 intensity;

    Light() : position(0, 0, 0), intensity(1, 1, 1) {}
    Light(const Vec3 &p, const Vec3 &i) : position(p), intensity(i) {}
};
