// -----------------------------------------------------------------------------
//  Archivo: scenes.hpp
//  Descripción: Declaraciones de constructores de escenas/cámaras de ejemplo.
//  `makeBaseScene` y `makeLibreScene` crean escenas listas para render con sus
//  materiales y luces, y `makeDefaultCamera`/`makeLibreCamera` ajustan la vista.
// -----------------------------------------------------------------------------
#pragma once

#include "rt/scene/scene.hpp"
#include "rt/core/camera.hpp"

Scene makeBaseScene(int maxDepth);
Camera makeDefaultCamera(int imageWidth, int imageHeight);

Scene makeLibreScene(int maxDepth);
Camera makeLibreCamera(int imageWidth, int imageHeight);
