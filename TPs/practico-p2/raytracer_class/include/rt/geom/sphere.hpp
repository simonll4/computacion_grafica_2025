// -----------------------------------------------------------------------------
//  Esfera con test de intersección analítico
// -----------------------------------------------------------------------------
//  Resuelve la intersección rayo-esfera mediante ecuación cuadrática.
//
//  Dado rayo r(t) = o + t·d y esfera con centro c y radio R:
//    ||r(t) - c||² = R²
//  Expandiendo:
//    ||o + t·d - c||² = R²
//    a·t² + 2b·t + c = 0
//  donde:
//    a = ||d||²         (siempre >0 si d normalizada → a≈1)
//    b = (o-c)·d        (proyección del offset sobre la dirección)
//    c = ||o-c||² - R²  (distancia al centro menos radio al cuadrado)
//
//  Discriminante Δ = b² - ac:
//    Δ < 0: sin intersección
//    Δ = 0: tangente (1 punto)
//    Δ > 0: dos puntos (entrada y salida)
//
//  Se elige la raíz más cercana (menor t) dentro de [t_min, t_max].
// -----------------------------------------------------------------------------

#pragma once

#include "rt/scene/object.hpp"

class Sphere : public Object
{
public:
    Vec3 center;   // Centro de la esfera
    double radius; // Radio de la esfera

    Sphere(const Vec3 &c, double r, const Material &m)
        : Object(m), center(c), radius(r) {}

    bool intersect(const Ray &ray, double t_min, double t_max, HitRecord &rec) const override
    {
        Vec3 oc = ray.origin - center; // Vector desde centro a origen del rayo
        
        // Coeficientes de la cuadrática a·t² + 2b·t + c = 0
        double a = dot(ray.direction, ray.direction);
        double half_b = dot(oc, ray.direction);
        double c = dot(oc, oc) - radius * radius;
        
        // Discriminante (usamos half_b para estabilidad numérica)
        double discriminant = half_b * half_b - a * c;
        if (discriminant < 0)
            return false; // No hay intersección
        
        double sqrtDiscriminant = std::sqrt(discriminant);
        
        // Prueba raíz más cercana: t = (-b - √Δ) / a
        double root = (-half_b - sqrtDiscriminant) / a;
        if (root < t_min || root > t_max)
        {
            // Prueba raíz más lejana: t = (-b + √Δ) / a
            root = (-half_b + sqrtDiscriminant) / a;
            if (root < t_min || root > t_max)
                return false; // Ambas raíces fuera del rango válido
        }
        
        // Llena el registro de intersección
        rec.t = root;
        rec.point = ray.at(rec.t);
        Vec3 outwardNormal = (rec.point - center) / radius; // Normal unitaria
        rec.setFaceNormal(ray, outwardNormal);
        rec.material = material;
        return true;
    }
};
