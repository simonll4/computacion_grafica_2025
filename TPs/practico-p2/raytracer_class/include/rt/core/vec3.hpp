// -----------------------------------------------------------------------------
//  Archivo: vec3.hpp
//  Descripción: Envoltura ligera sobre GLM para manejar vectores 3D. Se aliasa
//               Vec3 a glm::dvec3 y se exponen utilidades (dot, cross,
//               normalized, reflect, refract, clamp) para mantener el resto del
//               código prácticamente igual durante la migración matemática a GLM.
//  Nota (Consigna - Limitaciones): se utiliza únicamente GLM como librería
//  matemática; el resto del trazador (intersecciones/iluminación) es propio.
// -----------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/norm.hpp>
#include <iostream>

using Vec3 = glm::dvec3;

inline double length(const Vec3& v) {
    return glm::length(v);
}

inline double length2(const Vec3& v) {
    return glm::length2(v);
}

inline double dot(const Vec3& a, const Vec3& b) {
    return glm::dot(a, b);
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return glm::cross(a, b);
}

inline Vec3 normalized(const Vec3& v) {
    double len = length(v);
    if (len > 0.0) {
        return v / len;
    }
    return v;
}

inline Vec3 clamp(const Vec3& v, double minVal = 0.0, double maxVal = 1.0) {
    return glm::clamp(v, Vec3(minVal), Vec3(maxVal));
}

inline Vec3 reflect(const Vec3& i, const Vec3& n) {
    return glm::reflect(i, n);
}

inline bool refract(const Vec3& i, const Vec3& n, double eta_i, double eta_t, Vec3& refracted) {
    // glm::refract espera el cociente eta = eta_i / eta_t como parámetro.
    refracted = glm::refract(i, n, static_cast<double>(eta_i / eta_t));
    if (!std::isfinite(refracted.x) || !std::isfinite(refracted.y) || !std::isfinite(refracted.z)) {
        return false;
    }
    return true;
}

inline std::ostream& operator<<(std::ostream& os, const Vec3& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}
