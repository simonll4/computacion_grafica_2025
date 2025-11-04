// -----------------------------------------------------------------------------
//  Archivo: triangle.hpp
//  Descripción: Triángulo definido por vértices v0,v1,v2 con intersección
//  Möller–Trumbore. Resuelve r(t)=v0 + u·edge1 + v·edge2 con restricciones
//  0<=u, 0<=v, u+v<=1 y t dentro del rango. Rechaza paralelismo y puntos fuera
//  del triángulo mediante pruebas de baricéntricas (u,v).
// -----------------------------------------------------------------------------

#pragma once

#include "rt/scene/object.hpp"

class Triangle : public Object
{
public:
    Vec3 v0, v1, v2;
    Vec3 normal;

    Triangle(const Vec3 &a, const Vec3 &b, const Vec3 &c, const Material &m)
        : Object(m), v0(a), v1(b), v2(c)
    {
        normal = normalized(cross(v1 - v0, v2 - v0));
    }

    bool intersect(const Ray &ray, double t_min, double t_max, HitRecord &rec) const override
    {
        const double EPS = 1e-8;
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;
        Vec3 h = cross(ray.direction, edge2);
        double a = dot(edge1, h);
        if (std::abs(a) < EPS)
            return false;
        double f = 1.0 / a;
        Vec3 s = ray.origin - v0;
        double u = f * dot(s, h);
        if (u < 0.0 || u > 1.0)
            return false;
        Vec3 q = cross(s, edge1);
        double v = f * dot(ray.direction, q);
        if (v < 0.0 || (u + v) > 1.0)
            return false;
        double t = f * dot(edge2, q);
        if (t < t_min || t > t_max)
            return false;
        rec.t = t;
        rec.point = ray.at(t);
        rec.setFaceNormal(ray, normal);
        rec.material = material;
        return true;
    }
};
