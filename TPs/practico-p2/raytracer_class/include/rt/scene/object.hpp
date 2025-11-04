// -----------------------------------------------------------------------------
//  Archivo: object.hpp
//  Descripción: Definiciones base para objetos de escena e información de golpe
//  (HitRecord). Los objetos concretos (esfera, plano, triángulo) heredan de
//  `Object` e implementan `intersect` para poblar un `HitRecord` válido.
// -----------------------------------------------------------------------------

#pragma once

#include <memory>
#include "rt/core/ray.hpp"
#include "rt/scene/material.hpp"

struct HitRecord
{
    // Punto de impacto sobre la superficie (en espacio mundo).
    Vec3 point;
    // Normal orientada contra la dirección del rayo incidente.
    Vec3 normal;
    // Parámetro del rayo en el impacto (p = ray.at(t)).
    double t;
    // Verdadero si el rayo golpeó la cara "frontal" (entrando al objeto).
    bool frontFace;
    // Material del objeto en el punto de impacto.
    Material material;

    // Ajusta la orientación de la normal para que siempre se oponga al rayo.
    // Además determina si se trata de un impacto en la cara frontal o trasera.
    inline void setFaceNormal(const Ray &ray, const Vec3 &outwardNormal)
    {
        frontFace = dot(ray.direction, outwardNormal) < 0;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }
};

class Object
{
public:
    // Material del objeto (se copia al HitRecord en un impacto).
    Material material;
    explicit Object(const Material &m) : material(m) {}
    virtual ~Object() = default;
    // Debe escribir en `rec` si hay intersección válida dentro del rango [t_min, t_max].
    // Devuelve true si hay hit; en ese caso `rec.t` debe ser el más cercano.
    virtual bool intersect(const Ray &ray, double t_min, double t_max, HitRecord &rec) const = 0;
};

using ObjectPtr = std::shared_ptr<Object>;
