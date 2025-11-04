// -----------------------------------------------------------------------------
//  Archivo: ray.hpp
//  Descripción: Rayo 3D con origen y dirección. Permite evaluar puntos a lo
//  largo del rayo mediante `at(t) = origin + t * direction`.
// -----------------------------------------------------------------------------

#pragma once

#include "rt/core/vec3.hpp"

struct Ray
{
    Vec3 origin;
    Vec3 direction;

    Ray() : origin(), direction(0, 0, 1) {}
    Ray(const Vec3 &o, const Vec3 &d) : origin(o), direction(d) {}

    // Devuelve el punto a distancia paramétrica `t` sobre el rayo.
    Vec3 at(double t) const { return origin + direction * t; }
};
