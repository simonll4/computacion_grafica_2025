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
        // Proyecta la dirección del rayo sobre la normal; si es ~0, rayo ∥ plano y no hay cruce.
        double denom = dot(normal, ray.direction);
        const double EPS = 1e-8;
        if (std::abs(denom) < EPS)
            return false;
        // Despeje de la ecuación (point - o)·n = t·(d·n) ⇒ t = ((point - o)·n) / (d·n).
        double t = dot(point - ray.origin, normal) / denom;
        // Solo aceptamos parámetros dentro del rango visible del rayo.
        if (t < t_min || t > t_max)
            return false;
        // Carga de datos de la intersección en el hit record.
        rec.t = t;
        rec.point = ray.at(t);
        // Se orienta la normal acorde al sentido del rayo para manejar superficies dobles.
        rec.setFaceNormal(ray, normal);
        rec.material = material;
        return true;
    }
};
