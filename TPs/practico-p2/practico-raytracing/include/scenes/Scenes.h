// Scenes.h
#pragma once

#include <vector>
#include <string>
#include "raytracer/Scene.h"

namespace scenes {

using SceneFactory = Scene (*)();

struct SceneInfo {
    int id;               // 1..N
    const char* key;      // slug para filenames
    const char* name;     // descripción amigable
    SceneFactory create;  // fábrica
};

// Catálogo de escenas disponibles
const std::vector<SceneInfo>& all();

// Crear escena a partir de una elección del usuario (1..N). Si es inválida, retorna la principal.
Scene createByChoice(int choice);

// Información de la escena elegida (si inválida, retorna la principal)
const SceneInfo& infoForChoice(int choice);

// Fábricas concretas
Scene createPrincipalScene();
Scene createShadowTestScene();
Scene createMaterialTestScene();
Scene createAttenuationTestScene();
Scene createMultipleLightsScene();

} // namespace scenes
