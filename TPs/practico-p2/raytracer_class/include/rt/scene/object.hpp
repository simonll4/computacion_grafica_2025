// -----------------------------------------------------------------------------
//  Clase base para geometría y registro de intersección
// -----------------------------------------------------------------------------
//  Define la interfaz polimórfica Object y la estructura HitRecord.
//
//  HitRecord almacena toda la información necesaria tras una intersección:
//   - Punto de impacto
//   - Normal de superficie (orientada contra el rayo)
//   - Parámetro t del rayo
//   - Si el rayo entró por la cara frontal o trasera
//   - Material del objeto impactado
//
//  Object es la clase base para todas las geometrías (Sphere, Plane, etc.)
//  y define el método virtual intersect() que cada forma implementa.
// -----------------------------------------------------------------------------

#pragma once

#include <memory>
#include "rt/core/ray.hpp"
#include "rt/scene/material.hpp"

// Registro de intersección rayo-superficie
struct HitRecord
{
    Vec3 point;       // Punto de impacto en coordenadas mundo
    Vec3 normal;      // Normal de superficie orientada contra el rayo
    double t;         // Distancia paramétrica: point = ray.at(t)
    bool frontFace;   // true si rayo entró por cara frontal
    Material material; // Material del objeto en este punto

    // Configura la normal y determina si es cara frontal/trasera
    // outwardNormal: normal geométrica (apunta "hacia afuera" del objeto)
    inline void setFaceNormal(const Ray &ray, const Vec3 &outwardNormal)
    {
        frontFace = dot(ray.direction, outwardNormal) < 0;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }
};

// Clase base abstracta para objetos geométricos
class Object
{
public:
    Material material; // Material de todo el objeto (uniforme)
    
    explicit Object(const Material &m) : material(m) {}
    virtual ~Object() = default;
    
    // Test de intersección rayo-objeto
    // Debe escribir en 'rec' si encuentra intersección en [t_min, t_max]
    // Retorna true solo si hay hit válido
    virtual bool intersect(const Ray &ray, double t_min, double t_max, HitRecord &rec) const = 0;
};

using ObjectPtr = std::shared_ptr<Object>;
