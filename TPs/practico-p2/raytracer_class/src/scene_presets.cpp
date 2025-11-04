// Presets con los datos de escena y los builders que generan Scene/Camera.

#include "rt/scene/scene_presets.hpp"
#include "rt/scene/scene.hpp"
#include "rt/core/camera.hpp"
#include "rt/geom/sphere.hpp"
#include "rt/geom/plane.hpp"
#include "rt/geom/triangle.hpp"
#include "rt/scene/light.hpp"

#include <memory>

namespace rt::presets
{
    namespace
    {
        // Atajos para crear materiales con los parámetros que usa la consigna.
        Material makeDiffuse(const Vec3 &albedo, double ka, double kd, double ks,
                             double shininess, const Vec3 &specColour = Vec3(1.0))
        {
            Material mat(MaterialType::Diffuse, albedo);
            mat.ka = ka;
            mat.kd = kd;
            mat.ks = ks;
            mat.specularColour = specColour;
            mat.shininess = shininess;
            return mat;
        }

        Material makeMetal(const Vec3 &albedo, double fuzz, double ka, double kd,
                           double ks, double shininess, const Vec3 &specColour)
        {
            Material mat(MaterialType::Metal, albedo, fuzz);
            mat.ka = ka;
            mat.kd = kd;
            mat.ks = ks;
            mat.specularColour = specColour;
            mat.shininess = shininess;
            return mat;
        }

        Material makeDielectric(const Vec3 &albedo, double refractiveIndex,
                                double ka, double kd, double ks, double shininess,
                                const Vec3 &specColour = Vec3(1.0),
                                const Vec3 &absorption = Vec3(0.0),
                                double absorptionDistance = 0.0)
        {
            Material mat(MaterialType::Dielectric, albedo, 0.0, refractiveIndex, shininess,
                         ka, kd, ks, specColour, absorption, absorptionDistance);
            return mat;
        }
    }

    // Escena base: plano + tres esferas (difusa, metálica y dieléctrica).
    const ScenePreset kBaseScene = []()
    {
        ScenePreset preset("Base Scene", Vec3(0.7, 0.8, 1.0), 5);

        // Materiales empleados en la escena base.
        // Difuso rojo
        Material diffuseRed = makeDiffuse(Vec3(0.82, 0.26, 0.24), 0.12, 0.88, 0.05, 32.0);

        // Metal plateado
        Material metalSilver = makeMetal(Vec3(0.92, 0.92, 0.94), 0.02, 0.05, 0.1, 1.0, 96.0,
                                         Vec3(0.92, 0.92, 0.97));

        // Dieléctrico (vidrio): con absorción cálida sutil para efecto más realista
        // La absorción (0.004, 0.003, 0.002) da un leve tinte ámbar al atravesar el material
        Material dielectricGlass = makeDielectric(
            Vec3(1.0, 1.0, 1.0),       // albedo
            1.5,                       // índice de refracción (vidrio estándar)
            0.02,                      // ka (ambiente mínimo)
            0.05,                      // kd (difuso muy bajo)
            1.0,                       // ks (reflexión especular máxima)
            128.0,                     // shininess (highlight concentrado)
            Vec3(1.0, 1.0, 1.0),       // specColour (blanco puro)
            Vec3(0.004, 0.003, 0.002), // absorción cálida (leve ámbar)
            1.0                        // distancia de absorción
        );

        // Material del piso
        Material groundMat = makeDiffuse(Vec3(0.82, 0.82, 0.84), 0.12, 0.88, 0.03, 8.0);

        // Material de la pared de fondo: gris azulado oscuro para crear contraste
        // con el vidrio y hacer visible la refracción/reflexión
        Material backWallMat = makeDiffuse(Vec3(0.70, 0.75, 0.85), 0.1, 0.9, 0.02, 12.0);

        // Plano de piso y pared posterior visibles en la escena.
        preset.planes.emplace_back(Vec3(0, -1.0, 0), Vec3(0, 1, 0), groundMat);
        preset.planes.emplace_back(Vec3(0, 0, -7.0), Vec3(0, 0, 1), backWallMat);

        // Esferas con materiales difuso, metálico y dieléctrico.
        preset.spheres.emplace_back(Vec3(-1.5, 0.0, -4.5), 1.0, diffuseRed);
        preset.spheres.emplace_back(Vec3(2, 0.0, -4.0), 1.0, metalSilver);

        // Esfera de vidrio: posicionada más adelante (-2.5 vs -2.8) para dejar
        // espacio entre ella y la pared, haciendo más visible la refracción del fondo
        preset.spheres.emplace_back(Vec3(0.0, -0.3, -2.5), 0.7, dielectricGlass);

        // Luz principal: posicionada arriba y atrás del vidrio para crear
        // highlight especular nítido y realzar la refracción
        preset.lights.emplace_back(Vec3(0.0, 3.0, -3.0), Vec3(1.0, 1.0, 1.0));

        return preset;
    }();

    // Escena libre: objetos variados + triángulo para validar intersecciones.
    const ScenePreset kLibreScene = []()
    {
        ScenePreset preset("Libre Scene", Vec3(0.7, 0.8, 1.0), 5);

        // Materiales empleados en la escena libre.
        // Difuso rojo/albedo
        Material diffuseAlbedo = makeDiffuse(Vec3(0.78, 0.25, 0.2), 0.12, 0.9, 0.06, 24.0);

        // Difuso verde (acento)
        Material accentGreen = makeDiffuse(Vec3(0.25, 0.65, 0.38), 0.1, 0.9, 0.04, 20.0);

        // Metal dorado
        Material metalGold = makeMetal(Vec3(0.94, 0.82, 0.58), 0.1, 0.05, 0.15, 1.0, 96.0,
                                       Vec3(0.95, 0.82, 0.45));

        // Dieléctrico (vidrio)
        Material dielectricGlass = makeDielectric(Vec3(1.0, 1.0, 1.0), 1.5, 0.02, 0.05, 1.0, 96.0,
                                                  Vec3(0.98, 0.99, 1.0));

        // Material del piso
        Material groundMat = makeDiffuse(Vec3(0.9, 0.88, 0.85), 0.12, 0.88, 0.04, 10.0);

        // Material de la pared de fondo
        Material backWallMat = makeDiffuse(Vec3(0.76, 0.8, 0.86), 0.1, 0.9, 0.03, 14.0);

        // Escenografía base: piso y pared de fondo.
        preset.planes.emplace_back(Vec3(0, -1.0, 0), Vec3(0, 1, 0), groundMat);
        preset.planes.emplace_back(Vec3(0, 0, -7.0), Vec3(0, 0, 1), backWallMat);

        // Objetos con materiales variados y tamaños diferentes.
        preset.spheres.emplace_back(Vec3(-0.85, -0.6, -2.6), 0.4, diffuseAlbedo);
        preset.spheres.emplace_back(Vec3(0.35, -0.35, -3.8), 0.7, dielectricGlass);
        preset.spheres.emplace_back(Vec3(1.75, -0.2, -3.3), 0.85, metalGold);

        // Triángulo para testear intersección Möller–Trumbore.
        preset.triangles.emplace_back(Vec3(-3.4, -1.0, -4),
                                      Vec3(-2.1, -1.0, -4),
                                      Vec3(-2.75, 1.4, -4),
                                      accentGreen);

        // Tres luces con intensidades distintas para generar contraste.
        preset.lights.emplace_back(Vec3(-4.5, 4.5, -3.0), Vec3(1.0, 0.96, 0.92));
        preset.lights.emplace_back(Vec3(4.0, 3.0, 1.0), Vec3(0.35, 0.4, 0.5));
        preset.lights.emplace_back(Vec3(0.0, 6.0, -8.0), Vec3(0.45, 0.45, 0.45));

        return preset;
    }();

    // Cámara base: posicionada en ángulo para observar mejor el vidrio
    // La vista lateral realza la reflexión parcial y la refracción del fondo
    const CameraPreset kBaseCamera(
        Vec3(0.8, 0.9, 1.7),    // eye: ligeramente elevada y de costado
        Vec3(-0.2, -0.2, -3.5), // target: mira hacia el centro de la escena
        Vec3(0.0, 1.0, 0.0),    // up: vertical estándar
        55.0                    // vfov: FOV medio para menos distorsión
    );

    const CameraPreset kLibreCamera(
        Vec3(2.2, 1.4, 3.4),    // eye (lookFrom)
        Vec3(-0.2, -0.4, -4.0), // target (lookAt)
        Vec3(0.0, 1.0, 0.0),    // up (vUp)
        50.0                    // vfov
    );

    Scene buildScene(const ScenePreset &preset, int maxDepthOverride)
    {
        Scene scene;
        scene.backgroundColour = preset.backgroundColour;
        scene.maxDepth = (maxDepthOverride >= 0) ? maxDepthOverride : preset.defaultMaxDepth;

        // Reservar capacidad para evitar realocaciones
        size_t totalObjects = preset.planes.size() + preset.spheres.size() + preset.triangles.size();
        scene.objects.reserve(totalObjects);
        scene.lights.reserve(preset.lights.size());

        // Agregar planos
        for (const auto &spec : preset.planes)
        {
            scene.addObject(std::make_shared<Plane>(spec.point, spec.normal, spec.material));
        }

        // Agregar esferas
        for (const auto &spec : preset.spheres)
        {
            scene.addObject(std::make_shared<Sphere>(spec.center, spec.radius, spec.material));
        }

        // Agregar triángulos
        for (const auto &spec : preset.triangles)
        {
            scene.addObject(std::make_shared<Triangle>(spec.v0, spec.v1, spec.v2, spec.material));
        }

        // Agregar luces
        for (const auto &spec : preset.lights)
        {
            scene.addLight(Light(spec.position, spec.intensity));
        }

        return scene;
    }

    Camera buildCamera(const CameraPreset &preset, int width, int height)
    {
        double aspectRatio = static_cast<double>(width) / height;
        return Camera(preset.eye, preset.target, preset.up, preset.vfov, aspectRatio);
    }

} // namespace rt::presets
