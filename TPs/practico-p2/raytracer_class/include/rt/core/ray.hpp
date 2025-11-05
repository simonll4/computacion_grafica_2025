// -----------------------------------------------------------------------------
//  Estructura de rayo para trazado
// -----------------------------------------------------------------------------
//  Un rayo representa una semi-recta en 3D definida por:
//    r(t) = origin + t * direction
//  donde t >= 0 (solo se consideran puntos "hacia adelante" desde el origen).
// -----------------------------------------------------------------------------

#pragma once

#include "rt/core/vec3.hpp"

struct Ray
{
    Vec3 origin;     // Punto de origen del rayo (donde "nace")
    Vec3 direction;  // Dirección del rayo (no necesariamente unitaria)

    Ray() : origin(), direction(0, 0, 1) {}                       // Constructor por defecto: rayo en +Z
    Ray(const Vec3 &o, const Vec3 &d) : origin(o), direction(d) {} // Constructor parametrizado

    // Evalúa el punto del rayo a distancia paramétrica t: r(t) = o + t·d
    Vec3 at(double t) const { return origin + direction * t; }
};
