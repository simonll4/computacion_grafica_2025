// -----------------------------------------------------------------------------
// Punto de entrada del raytracer
// -----------------------------------------------------------------------------
//  Flujo:
//   1. Parseo de argumentos CLI (resolución opcional)
//   2. Creación de directorio output/
//   3. Construcción de escena y cámara
//   4. Renderizado con antialiasing
//   5. Escritura de imagen PPM
//
//  Uso:
//   ./build/raytracer              → 1920x1080 (default)
//   ./build/raytracer <W> <H>      → resolución personalizada
//
//  Escena:
//   - Cinco esferas: 3 principales (jade, plata, vidrio) + 2 mini (amarilla, magenta)
//   - Plano verde
//   - Cielo: gradiente naranja (sin geometría, color de fondo)
//   - Iluminación con 4 luces puntuales
// -----------------------------------------------------------------------------

#include <iostream>
#include <memory>
#include <filesystem>
#include <string>

#include "rt/io/image_ppm.hpp"
#include "rt/core/renderer.hpp"
#include "rt/scene/scenes.hpp"

int main(int argc, char *argv[])
{
    // Configuración de renderizado
    RenderSettings settings{1920, 1080, 10}; // 1080p, 10 samples/pixel

    // Parseo de resolución opcional
    if (argc == 3)
    {
        try
        {
            int width = std::stoi(argv[1]);
            int height = std::stoi(argv[2]);

            if (width > 0 && height > 0)
            {
                settings.width = width;
                settings.height = height;
            }
            else
            {
                std::cerr << "[WARN] Resolución inválida, usando 1920x1080\n";
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "[WARN] Error parseando resolución: " << e.what() << "\n";
        }
    }
    else if (argc != 1)
    {
        std::cerr << "[INFO] Uso: " << argv[0] << " [width height]\n";
    }

    int maxDepth = 5;

    std::filesystem::create_directories("output");

    // Renderizado
    std::cerr << "Renderizando escena base...\n";

    Scene scene = makeBaseScene(maxDepth);
    Camera camera = makeDefaultCamera(settings.width, settings.height);
    Image img = render(scene, camera, settings);

    img.writePPM("output/render.ppm");
    std::cerr << "✓ Imagen guardada en: output/render.ppm\n";

    return 0;
}
