// -----------------------------------------------------------------------------
//  Plano infinito con test de intersección
// -----------------------------------------------------------------------------
//  Un plano se define por un punto P₀ y una normal N (unitaria).
//  Cualquier punto P en el plano cumple: (P - P₀) · N = 0
//
//  Intersección rayo-plano:
//    Rayo: r(t) = o + t·d
//    Plano: (r(t) - P₀) · N = 0
//    
//  Resolviendo para t:
//    (o + t·d - P₀) · N = 0
//    t·(d·N) = (P₀ - o)·N
//    t = (P₀ - o)·N / (d·N)
//
//  Casos especiales:
//   - Si d·N ≈ 0: rayo paralelo al plano (sin intersección o infinitas)
//   - Si t < t_min o t > t_max: fuera del rango visible
// -----------------------------------------------------------------------------

#pragma once

#include "rt/scene/object.hpp"

class Plane : public Object
{
public:
    Vec3 point;  // Punto de referencia en el plano
    Vec3 normal; // Normal del plano (unitaria)

    Plane(const Vec3 &p, const Vec3 &n, const Material &m)
        : Object(m), point(p), normal(normalized(n)) {}

    bool intersect(const Ray &ray, double t_min, double t_max, HitRecord &rec) const override
    {
        // Proyección de la dirección del rayo sobre la normal
        double denom = dot(normal, ray.direction);
        
        // Si denom ≈ 0, el rayo es paralelo al plano
        const double EPS = 1e-8;
        if (std::abs(denom) < EPS)
            return false;
        
        // Calcula parámetro t de intersección
        double t = dot(point - ray.origin, normal) / denom;
        
        // Verifica que esté en el rango válido
        if (t < t_min || t > t_max)
            return false;
        
        // Llena registro de intersección
        rec.t = t;
        rec.point = ray.at(t);
        rec.setFaceNormal(ray, normal);
        rec.material = material;
        return true;
    }
};
