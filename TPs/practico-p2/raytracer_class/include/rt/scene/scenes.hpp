// -----------------------------------------------------------------------------
//  API pública para construcción de escenas
// -----------------------------------------------------------------------------
//  Funciones wrapper que delegan en el sistema de presets.
//
//  Escena base:
//   - Cinco esferas: 3 principales (jade, plata, vidrio) + 2 mini (amarilla, magenta)
//   - Plano verde
//   - Cielo: gradiente naranja (color de fondo cuando no hay intersección)
//   - Cuatro luces puntuales
//   - Cámara en origen
// -----------------------------------------------------------------------------
#pragma once

#include "rt/scene/scene.hpp"
#include "rt/core/camera.hpp"

Scene makeBaseScene(int maxDepth);
Camera makeDefaultCamera(int imageWidth, int imageHeight);
