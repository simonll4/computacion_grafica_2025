// -----------------------------------------------------------------------------
//  Definición de escenas y builders
// -----------------------------------------------------------------------------
//  Implementa:
//   - Factories de materiales: makeDiffuse, makeMetal, makeDielectric
//   - kBaseScene: cinco esferas + plano + cielo naranja + 4 luces
//   - kBaseCamera: cámara en origen mirando hacia -Z
//   - buildScene/buildCamera: convierten presets en instancias runtime
// -----------------------------------------------------------------------------

#include "rt/scene/scene_presets.hpp"
#include "rt/scene/scene.hpp"
#include "rt/core/camera.hpp"
#include "rt/geom/sphere.hpp"
#include "rt/geom/plane.hpp"
#include "rt/scene/light.hpp"

#include <memory>

namespace rt::presets
{
    namespace
    {
        // Factories para construir materiales con parámetros Phong completos
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
                         ka, kd, ks, specColour, absorption, absorptionDistance); // Configura tinte e índice
            return mat;                                                           // Devuelve el dieléctrico listo (resto de campos permanecen según constructor)
        }
    }

    // Escena base: cielo naranja, cinco esferas, plano y 4 luces
    const ScenePreset kBaseScene = []()
    {
        ScenePreset preset("Sunset Meadow", Vec3(0.92, 0.45, 0.28), 10); // Cielo naranja atardecer

        // Materiales

        // Suelo: verde medio
        Material grassMat = makeDiffuse( // Material difuso para el plano de suelo
            Vec3(0.18, 0.42, 0.25),      // albedo: verde medio natural
            0.14,                        // ka: un poco más de ambiente
            0.80,                        // kd: sigue siendo difuso dominante
            0.08,                        // ks: ligero brillo superficial
            20.0,                        // shininess: mate
            Vec3(0.88, 0.92, 0.86)       // specularColour: neutro
        );

        // Difuso jade
        Material diffuseJade = makeDiffuse( // Material para la esfera difusa verde
            Vec3(0.22, 0.62, 0.53),
            0.16, 0.76, 0.12, 38.0,
            Vec3(0.92, 0.96, 0.88));

        // Amarillo fuerte
        Material diffuseBrightYellow = makeDiffuse( // Material para esfera amarilla pequeña
            Vec3(1.00, 0.88, 0.05),
            0.22, 0.78, 0.14, 40.0,
            Vec3(1.00, 0.96, 0.70));

        // Magenta neón
        Material diffuseNeonMagenta = makeDiffuse( // Material para esfera magenta pequeña
            Vec3(0.95, 0.10, 0.80),
            0.20, 0.75, 0.18, 42.0,
            Vec3(0.95, 0.80, 1.00));

        // Metal plata
        Material metalSilver = makeMetal( // Material metálico para la esfera derecha
            Vec3(0.55, 0.55, 0.60),
            0.08,                  // fuzz mayor ⇒ highlight más ancho
            0.02,                  // ka muy bajo
            0.02,                  // kd casi nulo (metal)
            0.75,                  // ks menor ⇒ menos espejo puro
            120.0,                 // shininess menor ⇒ hotspot más blando
            Vec3(0.92, 0.94, 0.98) // especular frío con leve atenuación
        );

        // Vidrio con leve absorción
        Material dielectricGlass = makeDielectric( // Material dieléctrico para esfera central
            Vec3(1.0, 1.0, 1.0),                   // albedo
            1.5,                                   // IOR vidrio
            0.0, 0.0, 0.0,                         // ka, kd, ks: sin iluminación local
            500.0,                                 // shininess
            Vec3(1.0, 1.0, 1.0),                   // specular blanco
            Vec3(0.06, 0.03, 0.01),                // absorción cálida (R,G,B) → ámbar sutil
            2.0                                    // distancia de absorción (mira bien en esferas grandes)
        );

        // ====== Geometría ======
        preset.planes.emplace_back(Vec3(0.0, -1.0, 0.0), Vec3(0, 1, 0), grassMat); // Plano del suelo horizontal

        // Izquierda: Difusa jade
        preset.spheres.emplace_back(Vec3(-4.0, 0.0, -13.0), 1.05, diffuseJade); // Esfera difusa izquierda

        // Centro: Vidrio
        preset.spheres.emplace_back(Vec3(-1.0, 0.0, -13.0), 1.10, dielectricGlass); // Esfera central de vidrio

        // Derecha: Metal (plata)
        preset.spheres.emplace_back(Vec3(1.5, 0.1, -8.0), 1.10, metalSilver); // Esfera metálica derecha

        // Mini esferas
        preset.spheres.emplace_back(Vec3(-1.0, -0.64, -18.0), 0.36, diffuseBrightYellow); // Esfera pequeña amarilla
        preset.spheres.emplace_back(Vec3(-1.0, -0.80, -5.0), 0.20, diffuseNeonMagenta);   // Esfera pequeña magenta

        // ====== Iluminación cielo ======
        preset.lights.emplace_back(
            Vec3(-8.0, 6.0, -2.0), // sol sobre el horizonte a la izquierda
            Vec3(4.2, 2.9, 1.8)    // cálido (naranja)
        );
        preset.lights.emplace_back(
            Vec3(6.0, 3.0, 2.0), // luz de cielo opuesta (relleno)
            Vec3(0.6, 0.8, 1.3)  // azulada, baja intensidad
        );
        preset.lights.emplace_back(
            Vec3(-3.2, 1.3, -10.1), // rebote cálido rasante
            Vec3(0.7, 0.45, 0.35));
        preset.lights.emplace_back(
            Vec3(2.5, 4.5, -6.0), // rim sutil para el metal/vidrio
            Vec3(0.45, 0.55, 0.9));

        return preset; // Devuelve el preset completamente configurado
    }();

    // Cámara
    const CameraPreset kBaseCamera(
        Vec3(0.0, 0.0, 0.0), // eye: origen del sistema
        Vec3(0.0, 0.0, -1),  // target: alineado con -Z
        Vec3(0.0, 1.0, 0.0), // up: vertical
        35.0                 // vfov: más amplio para capturar todas las esferas
    );

    Scene buildScene(const ScenePreset &preset, int maxDepthOverride)
    {
        Scene scene;                                                                          // Escena runtime a devolver
        scene.backgroundColour = preset.backgroundColour;                                     // Copia el color de fondo
        scene.maxDepth = (maxDepthOverride >= 0) ? maxDepthOverride : preset.defaultMaxDepth; // Selecciona profundidad efectiva

        // Reservar capacidad para evitar realocaciones
        size_t totalObjects = preset.planes.size() + preset.spheres.size(); // Cantidad total de geometrías
        scene.objects.reserve(totalObjects);                                // Reserva memoria para objetos
        scene.lights.reserve(preset.lights.size());                         // Reserva para luces

        // Agregar planos
        for (const auto &spec : preset.planes) // Convierte cada especificación de plano en objeto concreto
        {
            scene.addObject(std::make_shared<Plane>(spec.point, spec.normal, spec.material));
        }

        // Agregar esferas
        for (const auto &spec : preset.spheres) // Crea objetos esfera a partir del preset
        {
            scene.addObject(std::make_shared<Sphere>(spec.center, spec.radius, spec.material));
        }

        // Agregar luces
        for (const auto &spec : preset.lights) // Copia cada luz puntual al contenedor final
        {
            scene.addLight(Light(spec.position, spec.intensity));
        }

        return scene; // Escena lista para renderizar
    }

    Camera buildCamera(const CameraPreset &preset, int width, int height)
    {
        double aspectRatio = static_cast<double>(width) / height;                      // ratio ancho/alto solicitado
        return Camera(preset.eye, preset.target, preset.up, preset.vfov, aspectRatio); // Instancia cámara con FOV del preset
    }

} // namespace rt::presets
