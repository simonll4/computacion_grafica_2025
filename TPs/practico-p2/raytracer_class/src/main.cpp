// -----------------------------------------------------------------------------
//  Archivo: main.cpp
//  Descripción: Punto de entrada del trazador de rayos (raytracer).
//
//  FLUJO GENERAL DEL PROGRAMA:
//   1. Configuración de parámetros de renderizado (resolución, SPP, profundidad)
//   2. Creación del directorio de salida
//   3. Renderizado de la escena BASE (consigna B)
//   4. Renderizado de la escena LIBRE (consigna A)
//   5. Escritura de archivos PPM a disco
//
//  USO (CLI):
//   - Sin argumentos:         ./build/raytracer               → 800x600
//   - Con resolución (px):    ./build/raytracer <W> <H>       → sólo cambia resolución
//   Nota: sólo se aceptan width y height por argumentos; SPP y profundidad se
//   editan en el código (RenderSettings y maxDepth).
//
//  ARQUITECTURA DEL RAYTRACER:
//   - main.cpp: orquesta el flujo, configura parámetros
//   - scenes.cpp: construye escenas y cámaras (delega a scene_presets.cpp)
//   - renderer.cpp: bucle de render, antialiasing, corrección gamma
//   - scene.cpp: trazado de rayos recursivo (Scene::traceRay)
//   - material_shading.cpp: iluminación local (Phong) y global (reflexión/refracción)
//   - geom/*.hpp: intersección rayo-geometría (esfera, plano, triángulo)
//
//  CONSIGNA:
//   - Requisito 1: rayos primarios generados con Camera::getRay
//   - Requisito 2: intersección con al menos 3 tipos de geometría
//   - Requisito 3: iluminación local con luces puntuales y sombras
//   - Requisito 4: antialiasing estocástico (múltiples muestras por píxel)
//   - Requisito 5: reflexión y refracción recursivas
//   - Requisito 6: tres tipos de materiales (difuso, metal, dieléctrico)
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
    // ═════════════════════════════════════════════════════════════════════════
    // CONFIGURACIÓN DE RENDERIZADO
    // ═════════════════════════════════════════════════════════════════════════

    // Parámetros de imagen:
    // - width, height: resolución en píxeles (800x600 = aspect ratio 4:3)
    // - samplesPerPixel (SPP): muestras por píxel para antialiasing
    //   SPP=1: sin AA (bordes dentados), SPP=4: AA básico, SPP=16+: AA suave
    RenderSettings settings{1920, 1080, 4};

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
                std::cerr << "[WARN] Resolución inválida (" << width << "x" << height
                          << "), usando valores por defecto 800x600.\n";
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "[WARN] No se pudo parsear la resolución: " << e.what()
                      << ". Usando valores por defecto 800x600.\n";
        }
    }
    else if (argc != 1)
    {
        std::cerr << "[INFO] Uso: " << argv[0] << " [width height]\n"
                  << "       Width y height deben ser enteros positivos. "
                  << "Aplicando resolución por defecto 800x600.\n";
    }

    // Profundidad máxima de trazado recursivo:
    // Controla cuántos rebotes (reflexión/refracción) puede hacer un rayo.
    // maxDepth=1: sin reflexiones, maxDepth=5: hasta 5 rebotes
    // Valores altos permiten múltiples reflexiones pero son más lentos.
    int maxDepth = 5;

    // Crea el directorio de salida si no existe
    std::filesystem::create_directories("output");

    // ═════════════════════════════════════════════════════════════════════════
    // RENDERIZADO DE ESCENA BASE (Consigna B)
    // ═════════════════════════════════════════════════════════════════════════
    // Escena con tres esferas que demuestran los tres tipos de materiales:
    // - Esfera roja: material difuso (mate, sin reflexiones)
    // - Esfera plateada: material metálico (refleja como espejo)
    // - Esfera transparente: material dieléctrico (vidrio con refracción)
    int rc = 0;
    {
        // Construye la escena base con objetos, luces y materiales
        // (definida en scene_presets.cpp, accedida vía scenes.cpp)
        Scene scene = makeBaseScene(maxDepth);

        // Construye la cámara: posición (0,0,0), mirando hacia -Z, FOV 90°
        // FOV amplio (90°) da perspectiva dramática con más distorsión
        Camera camera = makeDefaultCamera(settings.width, settings.height);

        // Renderiza la escena: lanza rayos, calcula iluminación, aplica AA
        // Este proceso puede tardar varios segundos dependiendo de SPP y resolución
        Image img = render(scene, camera, settings);

        // Escribe imagen en formato PPM (ASCII, fácil de leer/depurar)
        img.writePPM("output/base.ppm");
        std::cerr << "Wrote output/base.ppm\n";
    }

    // ═════════════════════════════════════════════════════════════════════════
    // RENDERIZADO DE ESCENA LIBRE (Consigna A)
    // ═════════════════════════════════════════════════════════════════════════
    // Escena más compleja con:
    // - Múltiples objetos con materiales variados
    // - Un triángulo (demuestra intersección Möller-Trumbore)
    // - Tres luces con colores/intensidades distintas
    // - Composición más artística y dinámica
    {
        // Construye la escena libre con geometría y luces variadas
        Scene scene = makeLibreScene(maxDepth);

        // Cámara alternativa: posición elevada y desplazada, FOV 50°
        // FOV angosto (50°) da efecto "telefoto" con menos distorsión
        Camera camera = makeLibreCamera(settings.width, settings.height);

        // Renderiza y guarda
        Image img = render(scene, camera, settings);
        img.writePPM("output/libre.ppm");
        std::cerr << "Wrote output/libre.ppm\n";
    }

    return rc;
}
