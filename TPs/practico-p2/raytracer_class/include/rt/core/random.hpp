// -----------------------------------------------------------------------------
//  Generación de números aleatorios thread-safe
// -----------------------------------------------------------------------------
//  Provee utilidades RNG para:
//   - Antialiasing (jitter de muestras por píxel)
//   - Rugosidad de metales (perturbación de reflexiones)
//   - Muestreo hemisférico (path tracing futuro)
//
//  Usa thread_local para permitir paralelización sin mutex.
// -----------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <cmath>
#include <random>
#include "rt/core/vec3.hpp"

namespace rtcore
{
    constexpr double kPi = 3.14159265358979323846;

    // Generador Mersenne Twister independiente por hilo
    inline thread_local std::mt19937 rng{std::random_device{}()};

    // Distribución uniforme en [0, 1)
    inline thread_local std::uniform_real_distribution<double> uni01(0.0, 1.0);

    // Devuelve número aleatorio en [0, 1)
    inline double random_double() { return uni01(rng); }

    // Genera punto aleatorio dentro de esfera unitaria usando método de rechazo
    inline Vec3 random_in_unit_sphere()
    {
        while (true)
        {
            // Genera punto en cubo [-1, 1]³
            Vec3 p(random_double() * 2.0 - 1.0,
                   random_double() * 2.0 - 1.0,
                   random_double() * 2.0 - 1.0);

            // Acepta solo si está dentro de la esfera unitaria
            if (length2(p) >= 1.0)
                continue;

            return p;
        }
    }

    // Genera vector unitario uniformemente distribuido en la esfera
    // Usa coordenadas esféricas para evitar el sesgo del método de rechazo
    inline Vec3 random_unit_vector()
    {
        double z = random_double() * 2.0 - 1.0;  // cos(θ) en [-1, 1]
    double a = random_double() * 2.0 * kPi; // φ en [0, 2π]
        double r = std::sqrt(std::max(0.0, 1.0 - z * z)); // sin(θ)
        return Vec3(r * std::cos(a), r * std::sin(a), z);
    }

} // namespace rtcore
