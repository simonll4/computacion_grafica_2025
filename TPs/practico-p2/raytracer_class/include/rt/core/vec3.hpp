// -----------------------------------------------------------------------------
//  Vectores 3D para el raytracer
// -----------------------------------------------------------------------------
//  Wrapper sobre GLM (única librería matemática permitida según consigna).
//  Expone operaciones vectoriales esenciales: producto punto/cruz, normalización,
//  reflexión y refracción. Usa double precision para minimizar errores numéricos.
// -----------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/norm.hpp>
#include <iostream>

using Vec3 = glm::dvec3; // Alias para vectores de doble precisión (x, y, z)

// Magnitud del vector: ||v|| = √(x² + y² + z²)
inline double length(const Vec3 &v)
{
    return glm::length(v);
}

// Magnitud al cuadrado: ||v||² = x² + y² + z² (evita raíz cuadrada costosa)
inline double length2(const Vec3 &v)
{
    return glm::length2(v);
}

// Producto punto: a·b = ax*bx + ay*by + az*bz
inline double dot(const Vec3 &a, const Vec3 &b)
{
    return glm::dot(a, b);
}

// Producto cruz: a×b (vector perpendicular a a y b)
inline Vec3 cross(const Vec3 &a, const Vec3 &b)
{
    return glm::cross(a, b);
}

// Normalización: v/||v|| (vector unitario en misma dirección)
inline Vec3 normalized(const Vec3 &v)
{
    double len = length(v);
    if (len > 0.0)
    {
        return v / len;
    }
    return v; // Vector nulo permanece sin cambios
}

// Clampea cada componente del vector al rango [minVal, maxVal]
inline Vec3 clamp(const Vec3 &v, double minVal = 0.0, double maxVal = 1.0)
{
    return glm::clamp(v, Vec3(minVal), Vec3(maxVal));
}

// Reflexión especular: R = i - 2(i·n)n (i: incidente, n: normal)
inline Vec3 reflect(const Vec3 &i, const Vec3 &n)
{
    return glm::reflect(i, n);
}

// Refracción (Ley de Snell): calcula rayo transmitido
// eta_i: índice refractivo origen, eta_t: índice refractivo destino
// Retorna false si hay reflexión total interna
inline bool refract(const Vec3 &i, const Vec3 &n, double eta_i, double eta_t, Vec3 &refracted)
{
    refracted = glm::refract(i, n, static_cast<double>(eta_i / eta_t));

    // Verifica que el resultado sea válido (sin NaN/infinitos)
    if (!std::isfinite(refracted.x) || !std::isfinite(refracted.y) || !std::isfinite(refracted.z))
    {
        return false;
    }

    // Magnitud nula indica reflexión total interna
    if (glm::length2(refracted) < 1e-12)
    {
        return false;
    }

    return true;
}

// Operador de salida para debug: imprime "(x, y, z)"
inline std::ostream &operator<<(std::ostream &os, const Vec3 &v)
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}
