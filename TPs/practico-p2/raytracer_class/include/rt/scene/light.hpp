// -----------------------------------------------------------------------------
//  Luz puntual para iluminación local
// -----------------------------------------------------------------------------
//  Define una fuente de luz puntual usada por el modelo de Phong.
//  La luz tiene posición fija en el espacio e intensidad RGB.
//  La atenuación por distancia se calcula en material_shading.cpp.
// -----------------------------------------------------------------------------

#pragma once

#include "rt/core/vec3.hpp"

struct Light
{
    Vec3 position;  // Posición de la luz en coordenadas mundo
    Vec3 intensity; // Intensidad/color de la luz (valores lineales, no gamma)

    Light() : position(0, 0, 0), intensity(1, 1, 1) {}
    Light(const Vec3 &p, const Vec3 &i) : position(p), intensity(i) {}
};
