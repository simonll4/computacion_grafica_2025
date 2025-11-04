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
        // Vector del centro al origen del rayo.
        // Se reutiliza en varias proyecciones para armar la cuadrática.
        Vec3 oc = ray.origin - center;
        // Coeficiente cuadrático a = ||d||^2 (dirección normalizada → a≈1).
        double a = dot(ray.direction, ray.direction);
        // half_b = b = (o-c)·d, forma más estable al evitar un factor 2.
        double half_b = dot(oc, ray.direction);
        // c = ||o-c||^2 - R^2, distancia al centro descontando el radio.
        double c = dot(oc, oc) - radius * radius;
        // discriminante = b^2 - ac, indica si hay dos, una o cero raíces reales.
        double discriminant = half_b * half_b - a * c;
        if (discriminant < 0)
            return false;
        double sqrtDiscriminant = std::sqrt(discriminant);
        // Primera raíz: t = (-b - √Δ) / a (la más cercana).
        double root = (-half_b - sqrtDiscriminant) / a;
        if (root < t_min || root > t_max)
        {
            // Si la más cercana cae fuera del rango, probamos la más lejana.
            root = (-half_b + sqrtDiscriminant) / a;
            if (root < t_min || root > t_max)
                return false;
        }
        rec.t = root;
        rec.point = ray.at(rec.t);
        // Normal geométrica = (p - c) / R. Se ajusta el sentido con setFaceNormal.
        Vec3 outwardNormal = (rec.point - center) / radius;
        rec.setFaceNormal(ray, outwardNormal);
        rec.material = material;
        return true;
    }
};
