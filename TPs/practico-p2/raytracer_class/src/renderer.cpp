// -----------------------------------------------------------------------------
// Bucle principal de renderizado con antialiasing
// -----------------------------------------------------------------------------
//  Implementa:
//   1. Antialiasing estocástico: N samples/pixel con jitter aleatorio
//   2. Coordenadas UV normalizadas: (i+rand)/width, (j+rand)/height
//   3. Trazado por Camera::getRay() → Scene::traceRay()
//   4. Corrección gamma 2.2: sRGB ≈ linear^(1/2.2)
//   5. Conversión a 8-bit RGB para PPM
// -----------------------------------------------------------------------------

#include <cmath>
#include <algorithm>
#include <iostream>

#include "rt/core/renderer.hpp"
#include "rt/core/random.hpp"

namespace
{
    // Convierte [0,1] → [0,255] (con clamp)
    inline unsigned char to_u8(double x)
    {
        x = std::max(0.0, std::min(1.0, x));
        return static_cast<unsigned char>(255.99 * x);
    }
}

Image render(const Scene &scene, const Camera &camera, const RenderSettings &settings)
{
    Image img(settings.width, settings.height);

    for (int j = settings.height - 1; j >= 0; --j)
    {
        std::cerr << "Scanlines remaining: " << j << "\r";

        for (int i = 0; i < settings.width; ++i)
        {
            Vec3 pixelColour(0.0);

            // Antialiasing: N samples con jitter aleatorio
            for (int s = 0; s < settings.samplesPerPixel; ++s)
            {
                // UV con jitter: (i+rand)/width, (j+rand)/height
                double u = (i + rtcore::random_double()) / (settings.width - 1);
                double v = (j + rtcore::random_double()) / (settings.height - 1);

                Ray r = camera.getRay(u, v);
                pixelColour += scene.traceRay(r, scene.maxDepth);
            }

            // Promedia y aplica gamma 2.2
            double scale = 1.0 / settings.samplesPerPixel;
            double rr = std::pow(std::max(0.0, std::min(1.0, pixelColour.x * scale)), 1.0 / 2.2);
            double gg = std::pow(std::max(0.0, std::min(1.0, pixelColour.y * scale)), 1.0 / 2.2);
            double bb = std::pow(std::max(0.0, std::min(1.0, pixelColour.z * scale)), 1.0 / 2.2);

            // Escribe píxel (PPM es top‑down: fila 0 arriba). Se invierte Y.
            img.put(i, settings.height - 1 - j, RGB(to_u8(rr), to_u8(gg), to_u8(bb)));
        }
    }

    std::cerr << "\n";
    return img;
}
