// -----------------------------------------------------------------------------
//  Archivo: renderer.cpp
//  Descripción: Bucle principal de renderizado. Implementa:
//   1. Antialiasing estocástico (muestreo múltiple por píxel con jitter)
//   2. Trazado de rayos primarios mediante Camera::getRay
//   3. Acumulación de color con Scene::traceRay
//   4. Corrección gamma (2.2) para espacio sRGB
//   5. Conversión a RGB de 8 bits para salida PPM
// -----------------------------------------------------------------------------

#include <cmath>
#include <algorithm>
#include <iostream>

#include "rt/core/renderer.hpp"
#include "rt/core/random.hpp"

namespace
{
    // Convierte un valor flotante [0,1] a entero sin signo de 8 bits [0,255].
    // Clampea el valor al rango válido antes de la conversión.
    inline unsigned char to_u8(double x)
    {
        x = std::max(0.0, std::min(1.0, x));
        return static_cast<unsigned char>(255.99 * x);
    }
}

/// Renderiza una escena completa con antialiasing estocástico.
/// @param scene Escena con objetos, luces y configuración de trazado
/// @param camera Cámara que define el punto de vista y proyección
/// @param settings Parámetros de render (resolución y SPP)
/// @return Imagen renderizada lista para escribir a disco
Image render(const Scene &scene, const Camera &camera, const RenderSettings &settings)
{
    // Crea la imagen de salida con las dimensiones especificadas
    Image img(settings.width, settings.height);

    // Recorre cada scanline de arriba hacia abajo (j decrece)
    for (int j = settings.height - 1; j >= 0; --j)
    {
        // Muestra progreso en stderr (no interfiere con stdout)
        std::cerr << "Scanlines remaining: " << j << "\r";
        
        // Recorre cada píxel de izquierda a derecha
        for (int i = 0; i < settings.width; ++i)
        {
            Vec3 pixelColour(0.0); // Acumulador de color para este píxel
            
            // Antialiasing: lanza múltiples rayos por píxel con posiciones aleatorias
            for (int s = 0; s < settings.samplesPerPixel; ++s)
            {
                // Coordenadas normalizadas [0,1] con jitter aleatorio para AA
                // El jitter distribuye las muestras dentro del píxel
                double u = (i + rtcore::random_double()) / (settings.width - 1);
                double v = (j + rtcore::random_double()) / (settings.height - 1);
                
                // Genera rayo primario desde la cámara hacia (u,v) en el viewport
                Ray r = camera.getRay(u, v);
                
                // Traza el rayo y acumula el color resultante
                // scene.maxDepth limita los rebotes recursivos (reflexión/refracción)
                pixelColour += scene.traceRay(r, scene.maxDepth);
            }
            
            // Promedia las muestras acumuladas
            double scale = 1.0 / settings.samplesPerPixel;
            
            // Aplica corrección gamma 2.2 para convertir de espacio lineal a sRGB
            // Esto hace que los colores se vean correctos en monitores estándar
            // La fórmula es: sRGB = linear^(1/2.2)
            double rr = std::pow(std::max(0.0, std::min(1.0, pixelColour.x * scale)), 1.0 / 2.2);
            double gg = std::pow(std::max(0.0, std::min(1.0, pixelColour.y * scale)), 1.0 / 2.2);
            double bb = std::pow(std::max(0.0, std::min(1.0, pixelColour.z * scale)), 1.0 / 2.2);
            
            // Escribe el píxel en la imagen (invierte Y porque PPM es top-down)
            img.put(i, settings.height - 1 - j, RGB(to_u8(rr), to_u8(gg), to_u8(bb)));
        }
    }

    std::cerr << "\n"; // Nueva línea después del progreso
    return img;
}
