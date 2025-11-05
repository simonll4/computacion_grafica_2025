// -----------------------------------------------------------------------------
//  Sistema de configuración de escenas mediante presets
// -----------------------------------------------------------------------------
//  Separa la definición de escenas (datos) de su construcción (lógica).
//
//  Estructura:
//   - Specs: describen objetos con material embebido (SphereSpec, PlaneSpec)
//   - CameraPreset: parámetros de cámara (posición, objetivo, FOV)
//   - ScenePreset: colección completa (objetos + luces + fondo)
//   - Builders: convierten presets en instancias Scene/Camera
//
//  Presets disponibles:
//   - kBaseScene: cinco esferas + plano + cielo naranja + 4 luces
//   - kBaseCamera: cámara en ángulo para observar efectos de vidrio
// -----------------------------------------------------------------------------

#pragma once

#include "rt/core/vec3.hpp"
#include "rt/scene/material.hpp"

#include <vector>
#include <string>

class Scene;
class Camera;

namespace rt::presets
{

// Especificaciones de geometría con material embebido

struct SphereSpec
{
    Vec3 center;
    double radius;
    Material material;

    SphereSpec(const Vec3 &c, double r, const Material &mat)
        : center(c), radius(r), material(mat) {}
};

struct PlaneSpec
{
    Vec3 point;  // Punto en el plano
    Vec3 normal; // Normal (define orientación)
    Material material;

    PlaneSpec(const Vec3 &p, const Vec3 &n, const Material &mat)
        : point(p), normal(n), material(mat) {}
};

struct LightSpec
{
    Vec3 position;
    Vec3 intensity; // Intensidad RGB

    LightSpec(const Vec3 &p, const Vec3 &i)
        : position(p), intensity(i) {}
};

// Configuración de cámara

struct CameraPreset
{
    Vec3 eye;    // Posición (lookFrom)
    Vec3 target; // Objetivo (lookAt)
    Vec3 up;     // Vector "arriba" (vUp)
    double vfov; // Campo de visión vertical (grados)

    CameraPreset(const Vec3 &e, const Vec3 &t, const Vec3 &u, double fov)
        : eye(e), target(t), up(u), vfov(fov) {}
};

// Configuración de escena completa

struct ScenePreset
{
    std::string name;
    Vec3 backgroundColour;           // Color de cielo
    int defaultMaxDepth;             // Profundidad de trazado recursivo
    std::vector<SphereSpec> spheres;
    std::vector<PlaneSpec> planes;
    std::vector<LightSpec> lights;

    ScenePreset(const std::string &n, const Vec3 &bg, int depth)
        : name(n), backgroundColour(bg), defaultMaxDepth(depth) {}
};

// Presets predefinidos

extern const ScenePreset kBaseScene;   // Cinco esferas con tres materiales distintos
extern const CameraPreset kBaseCamera; // Cámara en ángulo

// Builders: convierten presets en instancias

Scene buildScene(const ScenePreset &preset, int maxDepthOverride = -1);
Camera buildCamera(const CameraPreset &preset, int width, int height);

} // namespace rt::presets
