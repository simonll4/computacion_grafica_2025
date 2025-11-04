#include "rt/scene/scenes.hpp"
#include "rt/scene/scene_presets.hpp"

// Delega en el preset de cámara base
Camera makeDefaultCamera(int imageWidth, int imageHeight)
{
    return rt::presets::buildCamera(rt::presets::kBaseCamera, imageWidth, imageHeight);
}

// Delega en el preset de escena base
Scene makeBaseScene(int maxDepth)
{
    return rt::presets::buildScene(rt::presets::kBaseScene, maxDepth);
}

// Delega en el preset de cámara libre
Camera makeLibreCamera(int imageWidth, int imageHeight)
{
    return rt::presets::buildCamera(rt::presets::kLibreCamera, imageWidth, imageHeight);
}

// Delega en el preset de escena libre
Scene makeLibreScene(int maxDepth)
{
    return rt::presets::buildScene(rt::presets::kLibreScene, maxDepth);
}
