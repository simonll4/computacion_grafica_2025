// Scenes.cpp
#include "scenes/Scenes.h"

#include "raytracer/Sphere.h"
#include "raytracer/AABB.h"
#include "raytracer/Light.h"
#include "raytracer/Material.h"

#include <glm/glm.hpp>

namespace scenes {

// Escena principal (antes createTestScene())
Scene createPrincipalScene() {
    Scene scene;

    // Luz ambiental
    scene.setAmbientLight(glm::vec3(0.15f, 0.15f, 0.20f));

    // Materiales
    Material redMaterial(
        glm::vec3(0.1f, 0.05f, 0.05f),  // Ambiental
        glm::vec3(0.8f, 0.2f, 0.2f),    // Difuso (rojo)
        glm::vec3(0.9f, 0.9f, 0.9f),    // Especular
        64.0f                           // Shininess
    );

    Material blueMaterial(
        glm::vec3(0.05f, 0.05f, 0.1f),  // Ambiental
        glm::vec3(0.2f, 0.4f, 0.8f),    // Difuso (azul)
        glm::vec3(0.7f, 0.7f, 0.7f),    // Especular
        32.0f                           // Shininess
    );

    Material greenMaterial(
        glm::vec3(0.05f, 0.1f, 0.05f),  // Ambiental
        glm::vec3(0.2f, 0.8f, 0.2f),    // Difuso (verde)
        glm::vec3(0.5f, 0.5f, 0.5f),    // Especular
        16.0f                           // Shininess
    );

    Material yellowMaterial(
        glm::vec3(0.1f, 0.1f, 0.05f),   // Ambiental
        glm::vec3(0.8f, 0.8f, 0.2f),    // Difuso (amarillo)
        glm::vec3(0.6f, 0.6f, 0.6f),    // Especular
        24.0f                           // Shininess
    );

    // Objetos
    scene.addObject(std::make_shared<Sphere>(
        glm::vec3(-2.0f, 0.0f, -5.0f),  // Centro
        1.2f,                           // Radio
        redMaterial
    ));

    scene.addObject(std::make_shared<AABB>(
        glm::vec3(2.0f, 0.0f, -5.0f),   // Centro
        2.0f,                           // Tamaño
        blueMaterial
    ));

    scene.addObject(std::make_shared<Sphere>(
        glm::vec3(0.0f, -0.5f, -3.0f),  // Centro
        0.5f,                           // Radio
        greenMaterial
    ));

    // Cubo piso (AABB con min/max explícitos)
    scene.addObject(std::make_shared<AABB>(
        glm::vec3(0.0f, -3.0f, -5.0f),          // Min point
        glm::vec3(8.0f, -1.5f, -10.0f),         // Max point
        yellowMaterial
    ));

    // Luces puntuales
    Light mainLight(
        glm::vec3(0.0f, 5.0f, -2.0f),
        glm::vec3(1.0f, 1.0f, 1.0f)
    );
    mainLight.constant = 1.0f;
    mainLight.linear = 0.05f;
    mainLight.quadratic = 0.01f;
    scene.addLight(mainLight);

    Light sideLight(
        glm::vec3(5.0f, 2.0f, -4.0f),
        glm::vec3(0.7f, 0.6f, 0.5f)
    );
    sideLight.constant = 1.0f;
    sideLight.linear = 0.07f;
    sideLight.quadratic = 0.017f;
    scene.addLight(sideLight);

    Light fillLight(
        glm::vec3(-3.0f, -1.0f, 0.0f),
        glm::vec3(0.3f, 0.3f, 0.4f)
    );
    fillLight.constant = 1.0f;
    fillLight.linear = 0.09f;
    fillLight.quadratic = 0.032f;
    scene.addLight(fillLight);

    return scene;
}

// Escena 1: Validación de sombras
Scene createShadowTestScene() {
    Scene scene;
    scene.setAmbientLight(glm::vec3(0.1f));

    // Material blanco para ver mejor las sombras
    Material whiteMat(
        glm::vec3(0.1f),
        glm::vec3(0.9f),
        glm::vec3(0.3f),
        16.0f
    );

    // Piso grande
    scene.addObject(std::make_shared<AABB>(
        glm::vec3(-10.0f, -2.0f, -20.0f),
        glm::vec3(10.0f, -1.5f, 5.0f),
        whiteMat
    ));

    // Esfera que proyecta sombra
    scene.addObject(std::make_shared<Sphere>(
        glm::vec3(0.0f, 0.5f, -5.0f),
        1.0f,
        Material(glm::vec3(0.1f), glm::vec3(0.8f, 0.2f, 0.2f), glm::vec3(0.5f), 32.0f)
    ));

    // Cubo que proyecta sombra
    scene.addObject(std::make_shared<AABB>(
        glm::vec3(-2.0f, -0.5f, -7.0f),
        1.5f,
        Material(glm::vec3(0.1f), glm::vec3(0.2f, 0.2f, 0.8f), glm::vec3(0.5f), 32.0f)
    ));

    // Una sola luz fuerte para sombras duras
    Light light(glm::vec3(3.0f, 5.0f, -3.0f), glm::vec3(2.0f));
    light.constant = 1.0f;
    light.linear = 0.01f;
    light.quadratic = 0.001f;
    scene.addLight(light);

    return scene;
}

// Escena 2: Validación de materiales y brillos especulares
Scene createMaterialTestScene() {
    Scene scene;
    scene.setAmbientLight(glm::vec3(0.05f));

    // Diferentes valores de shininess
    float shininessValues[] = {4.0f, 16.0f, 64.0f, 256.0f};

    for (int i = 0; i < 4; ++i) {
        Material mat(
            glm::vec3(0.05f),
            glm::vec3(0.3f, 0.3f, 0.7f),  // Azul
            glm::vec3(0.9f),              // Alto especular
            shininessValues[i]
        );

        scene.addObject(std::make_shared<Sphere>(
            glm::vec3(-3.0f + i * 2.0f, 0.0f, -6.0f),
            0.8f,
            mat
        ));
    }

    // Luz frontal para ver los brillos
    scene.addLight(Light(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(1.5f)));

    return scene;
}

// Escena 3: Validación de atenuación por distancia
Scene createAttenuationTestScene() {
    Scene scene;
    scene.setAmbientLight(glm::vec3(0.02f));

    Material mat(
        glm::vec3(0.05f),
        glm::vec3(0.7f),
        glm::vec3(0.3f),
        32.0f
    );

    // Fila de esferas a diferentes distancias
    for (int i = 0; i < 5; ++i) {
        scene.addObject(std::make_shared<Sphere>(
            glm::vec3(0.0f, 0.0f, -3.0f - i * 2.0f),
            0.5f,
            mat
        ));
    }

    // Luz con fuerte atenuación
    Light light(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(2.0f));
    light.constant = 0.5f;
    light.linear = 0.2f;
    light.quadratic = 0.1f;
    scene.addLight(light);

    return scene;
}

// Escena 4: Múltiples luces de colores
Scene createMultipleLightsScene() {
    Scene scene;
    scene.setAmbientLight(glm::vec3(0.05f));

    // Esfera blanca en el centro
    scene.addObject(std::make_shared<Sphere>(
        glm::vec3(0.0f, 0.0f, -5.0f),
        1.5f,
        Material(glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(0.5f), 32.0f)
    ));

    // Cubo blanco
    scene.addObject(std::make_shared<AABB>(
        glm::vec3(3.0f, -0.5f, -6.0f),
        2.0f,
        Material(glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(0.5f), 32.0f)
    ));

    // Luz roja
    Light redLight(glm::vec3(-3.0f, 2.0f, -3.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    scene.addLight(redLight);

    // Luz verde
    Light greenLight(glm::vec3(0.0f, 4.0f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    scene.addLight(greenLight);

    // Luz azul
    Light blueLight(glm::vec3(3.0f, 1.0f, -3.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    scene.addLight(blueLight);

    return scene;
}

// Catálogo y utilidades
const std::vector<SceneInfo>& all() {
    static const std::vector<SceneInfo> kScenes = {
        {1, "escena_principal", "Escena principal (esfera + cubo + múltiples luces)", &createPrincipalScene},
        {2, "test_sombras", "Test de sombras", &createShadowTestScene},
        {3, "test_materiales", "Test de materiales (shininess)", &createMaterialTestScene},
        {4, "test_atenuacion", "Test de atenuación por distancia", &createAttenuationTestScene},
        {5, "test_multiples_luces", "Test de múltiples luces de colores", &createMultipleLightsScene},
    };
    return kScenes;
}

const SceneInfo& infoForChoice(int choice) {
    const auto& list = all();
    for (const auto& s : list) {
        if (s.id == choice) return s;
    }
    return list.front(); // por defecto, principal
}

Scene createByChoice(int choice) {
    return infoForChoice(choice).create();
}

} // namespace scenes
