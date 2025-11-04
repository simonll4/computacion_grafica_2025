// -----------------------------------------------------------------------------
//  Archivo: renderer.hpp
//  Descripción: Declaración del bucle de render y estructura de configuración.
//  `render` recorre la imagen, genera SPP rayos primarios por píxel mediante la
//  cámara, acumula color con `Scene::traceRay` y aplica corrección gamma.
// -----------------------------------------------------------------------------
#pragma once

#include "rt/io/image_ppm.hpp"
#include "rt/scene/scene.hpp"
#include "rt/core/camera.hpp"

struct RenderSettings {
    int width;            // ancho de imagen en píxeles
    int height;           // alto de imagen en píxeles
    int samplesPerPixel;  // muestras por píxel (AA estocástico)
};

// Realiza el render de la escena vista por la cámara con los parámetros dados.
Image render(const Scene& scene, const Camera& camera, const RenderSettings& settings);
