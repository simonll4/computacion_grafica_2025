// -----------------------------------------------------------------------------
//  Archivo: camera.hpp
//  Descripción: Cámara pinhole. Construye el sistema de ejes de vista (u,v,w)
//  a partir de (lookFrom, lookAt, vUp) y un tamaño de viewport definido por el
//  FOV vertical (`vfov`) y la relación de aspecto. Provee `getRay(s,t)` para
//  generar rayos primarios normalizados hacia coordenadas de imagen.
//
//  Detalles importantes:
//   - `w` apunta desde el objetivo hacia la cámara (dirección de vista opuesta).
//   - `u` es el eje horizontal del plano de imagen; `v` el vertical.
//   - `lowerLeftCorner` es la esquina inferior izquierda del viewport en el
//     espacio de cámara. `horizontal` y `vertical` son sus vectores base.
//   - `getRay(s,t)` interpola dentro del viewport: (0,0) = esquina inf. izq.,
//     (1,1) = esquina sup. der. y devuelve un rayo normalizado.
// -----------------------------------------------------------------------------

#pragma once

#include <cmath>

#include "rt/core/vec3.hpp"
#include "rt/core/ray.hpp"

inline double deg2rad(double degrees) { return degrees * M_PI / 180.0; }

class Camera
{
public:
    // Posición de la cámara en mundo.
    Vec3 origin;

    // Esquina inferior izquierda del viewport en mundo.
    Vec3 lowerLeftCorner;

    // Vector horizontal del viewport (ancho del plano de imagen).
    Vec3 horizontal;
    
    // Vector vertical del viewport (alto del plano de imagen).
    Vec3 vertical;

    // Parámetros:
    //  - lookFrom: posición de la cámara.
    //  - lookAt:   punto hacia el que mira la cámara.
    //  - vUp:      vector "arriba" para definir la orientación vertical.
    //  - vfov:     campo de visión vertical (grados).
    //  - aspectRatio: ancho/alto del viewport.
    Camera(const Vec3 &lookFrom, const Vec3 &lookAt, const Vec3 &vUp,
           double vfov, double aspectRatio)
    {
        double theta = deg2rad(vfov);
        double halfHeight = std::tan(theta / 2.0);
        double viewportHeight = 2.0 * halfHeight;
        double viewportWidth = aspectRatio * viewportHeight;
        Vec3 w = normalized(lookFrom - lookAt);
        Vec3 u = normalized(cross(vUp, w));
        Vec3 v = cross(w, u);
        origin = lookFrom;
        horizontal = viewportWidth * u;
        vertical = viewportHeight * v;
        lowerLeftCorner = origin - horizontal * 0.5 - vertical * 0.5 - w;
    }

    Ray getRay(double s, double t) const
    {
        // Interpola el punto dentro del viewport y apunta desde el origen.
        Vec3 dir = lowerLeftCorner + s * horizontal + t * vertical - origin;
        // Se normaliza para mantener longitudes de dirección coherentes.
        return Ray(origin, normalized(dir));
    }
};
