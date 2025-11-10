// -----------------------------------------------------------------------------
//  Cámara pinhole
// -----------------------------------------------------------------------------
//  Genera rayos primarios desde un punto (ojo de la cámara) hacia un plano de
//  imagen (viewport). El viewport se define mediante:
//   - FOV vertical (vfov): ángulo de visión en grados
//   - Aspect ratio: relación ancho/alto
//   - Sistema de coordenadas (u,v,w) construido a partir de lookFrom/lookAt/vUp
//
//  Sistema de ejes:
//   - w: apunta desde lookAt hacia lookFrom (dirección "atrás" de la cámara)
//   - u: eje horizontal (derecha) del viewport
//   - v: eje vertical (arriba) del viewport
//
//  La función getRay(s,t) genera un rayo para las coordenadas normalizadas
//  (s,t) en [0,1]×[0,1], donde (0,0) es esquina inferior izquierda y
//  (1,1) es esquina superior derecha del viewport.
// -----------------------------------------------------------------------------

#pragma once

#include <cmath>
#include "rt/core/vec3.hpp"
#include "rt/core/ray.hpp"

// Conversión grados → radianes
inline double deg2rad(double degrees)
{
    constexpr double kPi = 3.14159265358979323846;
    return degrees * kPi / 180.0;
}

class Camera
{
public:
    Vec3 origin;          // Posición de la cámara en el mundo
    Vec3 lowerLeftCorner; // Esquina inferior izquierda del viewport
    Vec3 horizontal;      // Vector que recorre el ancho del viewport
    Vec3 vertical;        // Vector que recorre el alto del viewport

    // Constructor: configura la cámara según parámetros de posicionamiento y FOV
    // lookFrom: posición del ojo
    // lookAt:   punto hacia el que mira la cámara
    // vUp:      vector "arriba" de referencia (ej: (0,1,0) para Y-up)
    // vfov:     campo de visión vertical en grados
    // aspectRatio: ancho/alto de la imagen (ej: 16/9 = 1.777...)
    Camera(const Vec3 &lookFrom, const Vec3 &lookAt, const Vec3 &vUp,
           double vfov, double aspectRatio)
    {
        // Calcula dimensiones del viewport en espacio 3D
        double theta = deg2rad(vfov);
        double halfHeight = std::tan(theta / 2.0);
        double viewportHeight = 2.0 * halfHeight;
        double viewportWidth = aspectRatio * viewportHeight;

        // Construye base ortonormal (u,v,w) de la cámara
        Vec3 w = normalized(lookFrom - lookAt); // Eje "atrás" (desde target a ojo)
        Vec3 u = normalized(cross(vUp, w));     // Eje "derecha"
        Vec3 v = cross(w, u);                   // Eje "arriba"

        // Define geometría del viewport
        origin = lookFrom;
        horizontal = viewportWidth * u;
        vertical = viewportHeight * v;
        lowerLeftCorner = origin - horizontal * 0.5 - vertical * 0.5 - w;
    }

    // Genera rayo primario hacia coordenadas (s,t) del viewport
    // s: horizontal en [0,1] (0=izquierda, 1=derecha)
    // t: vertical en [0,1] (0=abajo, 1=arriba)
    Ray getRay(double s, double t) const
    {
        Vec3 dir = lowerLeftCorner + s * horizontal + t * vertical - origin;
        return Ray(origin, normalized(dir));
    }
};
