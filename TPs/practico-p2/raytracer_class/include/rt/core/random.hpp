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
    // ═════════════════════════════════════════════════════════════════════════
    // Generadores de números aleatorios thread-safe
    // ═════════════════════════════════════════════════════════════════════════
    // Usa thread_local para que cada hilo tenga su propio generador,
    // evitando contención y permitiendo paralelización futura del renderer.

    /// Generador Mersenne Twister por hilo, inicializado con semilla aleatoria
    inline thread_local std::mt19937 rng{std::random_device{}()};

    /// Distribución uniforme en [0,1) para generar números aleatorios
    inline thread_local std::uniform_real_distribution<double> uni01(0.0, 1.0);

    /// Genera un número aleatorio uniforme en [0,1).
    /// Usado para jitter en antialiasing y muestreo estocástico.
    inline double random_double() { return uni01(rng); }

    /// Genera un punto aleatorio dentro de la esfera unitaria (radio 1).
    /// Método de rechazo: genera puntos en cubo [-1,1]³ y rechaza los que
    /// caen fuera de la esfera. Usado para perturbar reflexiones metálicas (fuzz).
    /// @return Vector con ||v|| < 1
    inline Vec3 random_in_unit_sphere()
    {
        while (true)
        {
            // Genera punto aleatorio en cubo [-1,1]³
            Vec3 p(random_double() * 2.0 - 1.0,
                   random_double() * 2.0 - 1.0,
                   random_double() * 2.0 - 1.0);

            // Rechaza si está fuera de la esfera unitaria
            if (length2(p) >= 1.0)
                continue;

            return p;
        }
    }

    /// Genera un vector unitario aleatorio uniformemente distribuido en la esfera.
    /// Usa coordenadas esféricas para muestreo uniforme (más eficiente que rechazo).
    /// @return Vector con ||v|| = 1
    inline Vec3 random_unit_vector()
    {
        // Genera coordenadas esféricas (θ, φ) uniformemente
        double z = random_double() * 2.0 - 1.0;  // cos(θ) ∈ [-1,1]
        double a = random_double() * 2.0 * M_PI; // φ ∈ [0,2π]

        // Convierte a cartesianas: x = r·sin(θ)·cos(φ), y = r·sin(θ)·sin(φ), z = r·cos(θ)
        double r = std::sqrt(std::max(0.0, 1.0 - z * z)); // sin(θ) = √(1-cos²θ)
        return Vec3(r * std::cos(a), r * std::sin(a), z);
    }

} // namespace rtcore
