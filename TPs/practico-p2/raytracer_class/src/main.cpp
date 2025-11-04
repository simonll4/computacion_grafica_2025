// -----------------------------------------------------------------------------
//  Archivo: main.cpp
//  Descripción: Punto de entrada del trazador de rayos.
//  Sin argumentos: renderiza SIEMPRE las dos escenas (base y libre) con
//  parámetros por defecto y escribe `output/base.ppm` y `output/libre.ppm`.
//
//  Flujo:
//   - Construcción de escenas y cámaras
//   - Llamada al renderer y guardado de imágenes PPM
//
//  Notas:
//   - La corrección gamma y el promedio por SPP se realizan en `renderer.cpp`.
//   - Las escenas definen materiales (difuso/metal/dieléctrico) y luces; la cámara
//     se ajusta para cada escena en `scenes.cpp`.
//
//  Consigna - Requisito 1: los rayos primarios se generan con Camera::getRay.

#include <iostream>
#include <memory>
#include <filesystem>

#include "rt/io/image_ppm.hpp"
#include "rt/core/renderer.hpp"
#include "rt/scene/scenes.hpp"

int main(int /*argc*/, char * /*argv*/[])
{
    // Parámetros de imagen y muestreo por defecto. `spp` controla AA;
    // `maxDepth` controla rebotes de reflexión/refracción.
    RenderSettings settings{800, 600, 4};
    int maxDepth = 5;

    // Asegura que exista el directorio de salida para escribir los .ppm
    std::filesystem::create_directories("output");

    // Renderiza ambas escenas de forma secuencial y escribe ambas salidas.
    int rc = 0;
    {
        // Escena base: tres materiales representativos y fondo visible.
        Scene scene = makeBaseScene(maxDepth);

        // Cámara por defecto alineada al eje Z- y FOV amplio.
        Camera camera = makeDefaultCamera(settings.width, settings.height);

        // Render y guardado.
        Image img = render(scene, camera, settings);
        img.writePPM("output/base.ppm");
        std::cerr << "Wrote output/base.ppm\n";
    }
    {
        // Escena libre: composición alternativa con más luces y variedad geométrica.
        Scene scene = makeLibreScene(maxDepth);

        // Cámara alternativa colocada fuera del origen con FOV más angosto.
        Camera camera = makeLibreCamera(settings.width, settings.height);

        // Render y guardado.
        Image img = render(scene, camera, settings);
        img.writePPM("output/libre.ppm");
        std::cerr << "Wrote output/libre.ppm\n";
    }
    return rc;
}
