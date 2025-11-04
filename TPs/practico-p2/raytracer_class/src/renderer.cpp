// -----------------------------------------------------------------------------
//  Archivo: renderer.cpp
//  Descripción: Implementación del renderer por muestreo estocástico.
//  Por cada píxel:
//   - Se generan `samplesPerPixel` rayos primarios con jitter (AA).
//   - Se acumula el color devolviendo `Scene::traceRay` por muestra.
//   - Se promedia y aplica corrección gamma (~2.2).
//  Se imprime progreso por scanline a stderr.
// -----------------------------------------------------------------------------

#include <cmath>
#include <algorithm>
#include <iostream>

#include "rt/core/renderer.hpp"
#include "rt/core/random.hpp"

namespace
{
    // Convierte un canal en [0,1] a entero 8-bit [0,255].
    // Se recorta (clamp) por seguridad y se escala por 255.99 para cubrir 255.
    inline unsigned char to_u8(double x)
    {
        x = std::max(0.0, std::min(1.0, x));
        return static_cast<unsigned char>(255.99 * x);
    }
}

Image render(const Scene &scene, const Camera &camera, const RenderSettings &settings)
{
    // Buffer de salida en memoria. Inicializa la imagen con el tamaño pedido.
    Image img(settings.width, settings.height);

    // Recorremos de arriba hacia abajo (convención habitual de render).
    for (int j = settings.height - 1; j >= 0; --j)
    {
        // Progreso simple a stderr (se sobreescribe la misma línea con \r).
        std::cerr << "Scanlines remaining: " << j << "\r";
        for (int i = 0; i < settings.width; ++i)
        {
            // Acumulador de color lineal (RGB en espacio lineal 0..1).
            Vec3 pixelColour(0.0);
            for (int s = 0; s < settings.samplesPerPixel; ++s)
            {
                // Jitter estocástico por submuestreo: AA por promedios.
                double u = (i + rtcore::random_double()) / (settings.width - 1);
                double v = (j + rtcore::random_double()) / (settings.height - 1);
                // Rayo primario a través de la cámara para (u,v).
                Ray r = camera.getRay(u, v);
                // Trazado recursivo: devuelve color lineal en [0, +inf) (se recorta luego).
                pixelColour += scene.traceRay(r, scene.maxDepth);
            }
            // Promedio por SPP y corrección gamma (aprox. monitor sRGB gamma~2.2).
            double scale = 1.0 / settings.samplesPerPixel;
            double rr = std::pow(std::max(0.0, std::min(1.0, pixelColour.x * scale)), 1.0 / 2.2);
            double gg = std::pow(std::max(0.0, std::min(1.0, pixelColour.y * scale)), 1.0 / 2.2);
            double bb = std::pow(std::max(0.0, std::min(1.0, pixelColour.z * scale)), 1.0 / 2.2);
            // `put` escribe con origen (0,0) en la esquina superior-izquierda.
            img.put(i, settings.height - 1 - j, RGB(to_u8(rr), to_u8(gg), to_u8(bb)));
        }
    }

    // Salto de línea para terminar la barra de progreso.
    std::cerr << "\n";
    return img;
}
