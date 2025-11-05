// -----------------------------------------------------------------------------
//  Bucle principal de renderizado
// -----------------------------------------------------------------------------
//  Define la estructura de configuración y la firma de la función render.
//  El renderer itera sobre píxeles, lanza rayos primarios con antialiasing
//  estocástico (jitter), acumula color y aplica corrección gamma.
// -----------------------------------------------------------------------------
#pragma once

#include "rt/io/image_ppm.hpp"
#include "rt/scene/scene.hpp"
#include "rt/core/camera.hpp"

struct RenderSettings {
    int width;            // Ancho de la imagen en píxeles
    int height;           // Alto de la imagen en píxeles
    int samplesPerPixel;  // Muestras por píxel para antialiasing (SPP)
};

// Renderiza la escena completa y devuelve la imagen resultante
Image render(const Scene& scene, const Camera& camera, const RenderSettings& settings);
