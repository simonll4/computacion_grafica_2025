// -----------------------------------------------------------------------------
//  Archivo: sphere.hpp
//  Descripción: Esfera con test de intersección mediante resolución de la
//  cuadrática. Dado un rayo r(t) = o + t·d, y centro c, radio R:
//       ||o + t·d - c||^2 = R^2
//  que lleva a: a·t^2 + 2b·t + c = 0 con
//       a = dot(d,d), b = dot(o-c, d), c = dot(o-c,o-c) - R^2.
//  Se usa la forma con `half_b = b` para estabilidad: discriminante = b^2 - a·c.
//  Se elige la raíz más pequeña dentro de [t_min, t_max].
// -----------------------------------------------------------------------------

#pragma once

#include "rt/scene/object.hpp"

class Sphere : public Object
{
public:
    Vec3 center;
    double radius;

    Sphere(const Vec3 &c, double r, const Material &m)
        : Object(m), center(c), radius(r) {}

    bool intersect(const Ray &ray, double t_min, double t_max, HitRecord &rec) const override
    {
        Vec3 oc = ray.origin - center;
        double a = dot(ray.direction, ray.direction);
        double half_b = dot(oc, ray.direction);
        double c = dot(oc, oc) - radius * radius;
        double discriminant = half_b * half_b - a * c;
        if (discriminant < 0)
            return false;
        double sqrtDiscriminant = std::sqrt(discriminant);
        double root = (-half_b - sqrtDiscriminant) / a;
        if (root < t_min || root > t_max)
        {
            root = (-half_b + sqrtDiscriminant) / a;
            if (root < t_min || root > t_max)
                return false;
        }
        rec.t = root;
        rec.point = ray.at(rec.t);
        Vec3 outwardNormal = (rec.point - center) / radius;
        rec.setFaceNormal(ray, outwardNormal);
        rec.material = material;
        return true;
    }
};
