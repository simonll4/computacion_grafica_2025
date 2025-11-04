// -----------------------------------------------------------------------------
//  Archivo: random.hpp
//  Descripción: Utilidades RNG thread_local. Se usan para:
//   - Jitter de submuestreo por píxel (AA) en el renderer.
//   - Perturbación de reflexión en metales con `fuzz` (direcciones aleatorias).
//   - Generación de vectores unitarios aleatorios (útiles para muestreos).
// -----------------------------------------------------------------------------

#pragma once

#include <random>
#include "rt/core/vec3.hpp"

namespace rtcore
{

    // RNG por hilo para uso seguro en paralelo.
inline thread_local std::mt19937 rng{std::random_device{}()}; // generador por hilo
    inline thread_local std::uniform_real_distribution<double> uni01(0.0, 1.0);

    inline double random_double() { return uni01(rng); }

    inline Vec3 random_in_unit_sphere()
    {
        while (true)
        {
            Vec3 p(random_double() * 2.0 - 1.0,
                   random_double() * 2.0 - 1.0,
                   random_double() * 2.0 - 1.0);
            if (length2(p) >= 1.0)
                continue;
            return p;
        }
    }

    inline Vec3 random_unit_vector()
    {
        double z = random_double() * 2.0 - 1.0; // cos(θ)
        double a = random_double() * 2.0 * M_PI;
        double r = std::sqrt(std::max(0.0, 1.0 - z * z));
        return Vec3(r * std::cos(a), r * std::sin(a), z);
    }

} // namespace rtcore
