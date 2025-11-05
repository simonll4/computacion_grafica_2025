// -----------------------------------------------------------------------------
//  API wrapper para construcción de escenas
// -----------------------------------------------------------------------------
//  Funciones que delegan en el sistema de presets para mantener la
//  configuración centralizada en scene_presets.cpp.
// -----------------------------------------------------------------------------

#include "rt/scene/scenes.hpp"
#include "rt/scene/scene_presets.hpp"

Camera makeDefaultCamera(int imageWidth, int imageHeight)
{
    return rt::presets::buildCamera(rt::presets::kBaseCamera, imageWidth, imageHeight);
}

Scene makeBaseScene(int maxDepth)
{
    return rt::presets::buildScene(rt::presets::kBaseScene, maxDepth);
}
