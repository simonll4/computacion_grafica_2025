// -----------------------------------------------------------------------------
//  Archivo: plane.hpp
//  Descripción: Plano infinito definido por un punto `point` y una normal `normal`.
//  Intersección con r(t) = o + t·d: se resuelve
//      t = dot(point - o, n) / dot(n, d)
//  si el denominador no es ~0 (ray paralelo). Valida `t` en [t_min, t_max].
// -----------------------------------------------------------------------------

#pragma once

#include "rt/scene/object.hpp"

class Plane : public Object
{
public:
    Vec3 point;
    Vec3 normal;

    Plane(const Vec3 &p, const Vec3 &n, const Material &m)
        : Object(m), point(p), normal(normalized(n)) {}

    bool intersect(const Ray &ray, double t_min, double t_max, HitRecord &rec) const override
    {
        double denom = dot(normal, ray.direction);
        const double EPS = 1e-8;
        if (std::abs(denom) < EPS)
            return false;
        double t = dot(point - ray.origin, normal) / denom;
        if (t < t_min || t > t_max)
            return false;
        rec.t = t;
        rec.point = ray.at(t);
        rec.setFaceNormal(ray, normal);
        rec.material = material;
        return true;
    }
};
