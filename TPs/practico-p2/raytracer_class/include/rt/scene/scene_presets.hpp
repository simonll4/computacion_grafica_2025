// -----------------------------------------------------------------------------
//  Archivo: scene_presets.hpp
//  Descripción: Definiciones de presets de escena inmutables. Separa la
//  configuración de escenas (materiales, geometría, luces, cámara) de su
//  construcción, permitiendo centralizar y reutilizar configuraciones.
//
//  Estructura:
//   - Structs de especificación (SphereSpec, PlaneSpec, etc.) que describen
//     cada objeto con su material embebido.
//   - CameraPreset: parámetros de cámara (posición, objetivo, FOV).
//   - ScenePreset: colección completa de objetos, luces y configuración.
//   - Funciones builder que convierten presets en objetos Scene/Camera.
// -----------------------------------------------------------------------------

#pragma once

#include "rt/core/vec3.hpp"
#include "rt/scene/material.hpp"

#include <vector>
#include <string>

// Forward declarations para reducir acoplamiento
class Scene;
class Camera;

namespace rt::presets
{

// Datos de geometría y luces con su material asociado.

/// Especificación de una esfera: centro, radio y material.
struct SphereSpec
{
    Vec3 center;
    double radius;
    Material material;

    SphereSpec(const Vec3 &c, double r, const Material &mat)
        : center(c), radius(r), material(mat) {}
};

/// Especificación de un plano: punto en el plano, normal y material.
struct PlaneSpec
{
    Vec3 point;
    Vec3 normal;
    Material material;

    PlaneSpec(const Vec3 &p, const Vec3 &n, const Material &mat)
        : point(p), normal(n), material(mat) {}
};

/// Especificación de un triángulo: tres vértices y material.
struct TriangleSpec
{
    Vec3 v0, v1, v2;
    Material material;

    TriangleSpec(const Vec3 &a, const Vec3 &b, const Vec3 &c, const Material &mat)
        : v0(a), v1(b), v2(c), material(mat) {}
};

/// Especificación de una luz puntual: posición e intensidad RGB.
struct LightSpec
{
    Vec3 position;
    Vec3 intensity;

    LightSpec(const Vec3 &p, const Vec3 &i)
        : position(p), intensity(i) {}
};

// Parámetros de la cámara asociados a un preset.

/// Preset de cámara: define posición, objetivo, orientación y FOV.
struct CameraPreset
{
    Vec3 eye;       ///< Posición de la cámara (lookFrom).
    Vec3 target;    ///< Punto hacia el que mira (lookAt).
    Vec3 up;        ///< Vector "arriba" para orientación (vUp).
    double vfov;    ///< Campo de visión vertical en grados.

    CameraPreset(const Vec3 &e, const Vec3 &t, const Vec3 &u, double fov)
        : eye(e), target(t), up(u), vfov(fov) {}
};

// Escena completa con fondo, materiales y lista de elementos.

/// Preset de escena: agrupa toda la configuración de una escena (objetos,
/// luces, fondo, profundidad de trazado).
struct ScenePreset
{
    std::string name;                   ///< Nombre descriptivo del preset.
    Vec3 backgroundColour;              ///< Color de fondo (cielo).
    int defaultMaxDepth;                ///< Profundidad máxima de rebotes.
    std::vector<SphereSpec> spheres;    ///< Lista de esferas.
    std::vector<PlaneSpec> planes;      ///< Lista de planos.
    std::vector<TriangleSpec> triangles;///< Lista de triángulos.
    std::vector<LightSpec> lights;      ///< Lista de luces puntuales.

    ScenePreset(const std::string &n, const Vec3 &bg, int depth)
        : name(n), backgroundColour(bg), defaultMaxDepth(depth) {}
};

// Presets que expone la entrega (definidos en scene_presets.cpp).

/// Preset de escena base: tres esferas (difusa, metal, dieléctrico) y planos.
extern const ScenePreset kBaseScene;

/// Preset de escena libre: composición alternativa con triángulo y múltiples luces.
extern const ScenePreset kLibreScene;

/// Preset de cámara para escena base: FOV amplio, vista frontal.
extern const CameraPreset kBaseCamera;

/// Preset de cámara para escena libre: FOV angosto, vista en perspectiva.
extern const CameraPreset kLibreCamera;

// Builders que instancian los tipos runtime a partir de los datos.

/// Construye una Scene a partir de un ScenePreset.
/// @param preset Preset de escena con todos los objetos y luces.
/// @param maxDepthOverride Profundidad máxima de rebotes (usa preset.defaultMaxDepth si <0).
/// @return Scene completamente inicializada.
Scene buildScene(const ScenePreset &preset, int maxDepthOverride = -1);

/// Construye una Camera a partir de un CameraPreset.
/// @param preset Preset de cámara con posición, objetivo y FOV.
/// @param width Ancho de imagen en píxeles.
/// @param height Alto de imagen en píxeles.
/// @return Camera configurada con el aspect ratio correcto.
Camera buildCamera(const CameraPreset &preset, int width, int height);

} // namespace rt::presets
