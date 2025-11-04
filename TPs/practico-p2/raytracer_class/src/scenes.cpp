// Implementaciones de escenas y cámaras separadas del main
// Consigna - Escena base y escena libre:
//  - Base: plano de fondo + tres esferas (difusa, metálica, dieléctrica)
//  - Libre: composición alternativa con al menos 3 objetos y un plano
//
// Guía rápida:
// - Sistema de ejes (unidades arbitrarias):
//    X: izquierda (-) ↔ derecha (+)
//    Y: abajo (-)     ↔ arriba   (+)
//    Z: hacia vos (+) ↔ al fondo (-)
// - La cámara mira hacia Z negativo.
// - ka/kd/ks/shininess: términos del modelo Phong (ambiente/difuso/especular y brillo).
//
// Tuning rápido (materiales/cámara/luces):
// - Difusos: ka≈0.05–0.15 (ambiente), kd≈0.7–1.0 (cantidad de color mate),
//   ks bajo (0.0–0.08) y shininess bajo/medio (8–32) para brillos suaves.
// - Metales: ks≈1.0, kd bajo (0–0.2), shininess alto (64–128+) y `fuzz`∈[0,0.3]
//   para controlar borrosidad de la reflexión. `specularColour` tiñe el reflejo.
// - Dieléctricos: refractiveIndex≈1.0–2.5 (aire 1.0, agua 1.33, vidrio 1.5),
//   ks≈1.0, kd muy bajo. `absorption` y `absorptionDistance` aplican tinte
//   volumétrico (Beer–Lambert) en el rayo transmitido.
// - Cámara: FOV alto (80–100°) = perspectiva amplia; FOV bajo (35–60°) = más zoom.
//   Mover `lookFrom` a lo largo de Z cambia la distancia aparente.
// - Luces: `intensity` es lineal RGB (no sRGB). Más intensidad o menor distancia
//   incrementan brillo por la atenuación cuadrática en `Scene::traceRay`.

#include "rt/scene/scenes.hpp"
#include "rt/scene/material.hpp"
#include "rt/scene/light.hpp"

#include "rt/geom/sphere.hpp"
#include "rt/geom/triangle.hpp"
#include "rt/geom/plane.hpp"

namespace
{
    // Relación de aspecto: ancho/alto. Útil para colocar correctamente el "viewport" de la cámara.
    inline double aspect(int w, int h) { return static_cast<double>(w) / h; }
}

Camera makeDefaultCamera(int imageWidth, int imageHeight)
{
    // Cámara básica para la escena "base":
    // - Posición: (0,0,0) mirando a (0,0,-1) → vista frontal.
    // - FOV vertical 90°: perspectiva amplia (más distorsión en bordes).
    // - El aspect ratio construye correctamente el tamaño del viewport.
    double aspectRatio = aspect(imageWidth, imageHeight);
    Vec3 lookFrom(0.0, 0.0, 0.0);
    Vec3 lookAt(0.0, 0.0, -1.0);
    Vec3 vUp(0.0, 1.0, 0.0);
    double vfov = 90.0;
    return Camera(lookFrom, lookAt, vUp, vfov, aspectRatio);
}

// Consigna (Escena B - Base): tres esferas con materiales distintos y un plano
// de fondo visible. Se ajusta `scene.maxDepth`.
Scene makeBaseScene(int maxDepth)
{
    Scene scene;
    // Color de fondo. Si un rayo no golpea nada, se mezcla con blanco.
    scene.backgroundColour = Vec3(0.7, 0.8, 1.0);
    // Límite de rebotes recursivos
    scene.maxDepth = maxDepth;

    // ───────────────────────────────── Materiales ─────────────────────────────────
    // Difuso = mate. El color sale principalmente por la componente difusa (kd * albedo).
    // Ajustes: subí kd para colores más vivos; bajá ks/shininess para brillos suaves.
    Material diffuseRed(MaterialType::Diffuse, Vec3(0.82, 0.26, 0.24));
    diffuseRed.ka = 0.12;
    diffuseRed.kd = 0.88;
    diffuseRed.ks = 0.05;
    diffuseRed.specularColour = Vec3(1.0);
    diffuseRed.shininess = 32.0;

    // Metal = refleja como espejo. `fuzz` agrega rugosidad (0 = espejo perfecto, >0 = borroso).
    // Ajustes: aumentar `fuzz` para reflejos más difusos; cambiar `specularColour` para tono metálico.
    Material metalSilver(MaterialType::Metal, Vec3(0.92, 0.92, 0.94), 0.02, 1.0, 96.0);
    metalSilver.ka = 0.05;
    metalSilver.kd = 0.1;
    metalSilver.ks = 1.0;
    metalSilver.specularColour = Vec3(0.92, 0.92, 0.97);

    // Dieléctrico = vidrio/agua. `refractiveIndex` controla cuánto se dobla la luz (1.5 ≈ vidrio).
    // Ajustes: `absorption`>0 con `absorptionDistance`>0 aplica tinte: valores altos = más absorción.
    Material dielectricGlass(MaterialType::Dielectric, Vec3(1.0, 1.0, 1.0), 0.0, 1.5, 96.0);
    dielectricGlass.ka = 0.02;
    dielectricGlass.kd = 0.05;
    dielectricGlass.ks = 1.0;
    dielectricGlass.specularColour = Vec3(1.0);
    dielectricGlass.absorption = Vec3(0.0);
    dielectricGlass.absorptionDistance = 0.0;

    // Materiales difusos para el piso y la pared de fondo (actúan como "escenografía").
    Material groundMat(MaterialType::Diffuse, Vec3(0.82, 0.82, 0.84));
    groundMat.ka = 0.12;
    groundMat.kd = 0.88;
    groundMat.ks = 0.03;
    groundMat.specularColour = Vec3(1.0);
    groundMat.shininess = 8.0;

    Material backWallMat(MaterialType::Diffuse, Vec3(0.85, 0.85, 0.88));
    backWallMat.ka = 0.1;
    backWallMat.kd = 0.9;
    backWallMat.ks = 0.02;
    backWallMat.specularColour = Vec3(1.0);
    backWallMat.shininess = 12.0;

    // ───────────────────────────────── Geometría ─────────────────────────────────
    // Plano de piso y pared de fondo (fondo visible):
    //  - Un plano se define con un punto y una normal. El "frente" del plano es el lado donde apunta la normal.
    //  - Piso: punto (0,-1,0), normal (0,1,0) → queda a la altura Y = -1 y "mira" hacia arriba.
    scene.addObject(std::make_shared<Plane>(Vec3(0, -1.0, 0), Vec3(0, 1, 0), groundMat));
    //  - Pared de fondo: punto (0,0,-7), normal (0,0,1) → pared a Z = -7 que "mira" hacia la cámara.
    scene.addObject(std::make_shared<Plane>(Vec3(0, 0, -7.0), Vec3(0, 0, 1), backWallMat));

    // Tres esferas con distintos materiales:
    //  - Coordenadas (x,y,z): x izquierda/derecha, y altura, z profundidad.
    scene.addObject(std::make_shared<Sphere>(Vec3(-1.5, 0.0, -4.5), 1.0, diffuseRed));
    scene.addObject(std::make_shared<Sphere>(Vec3(1.5, 0.0, -4.0), 1.0, metalSilver));
    scene.addObject(std::make_shared<Sphere>(Vec3(0.0, -0.3, -2.8), 0.7, dielectricGlass));

    // ───────────────────────────────── Iluminación ─────────────────────────────────
    // Al menos una luz puntual (como una "lamparita" en el espacio). La intensidad es RGB.
    scene.addLight(Light(Vec3(-5.0, 5.0, -2.0), Vec3(1.0, 1.0, 1.0)));
    return scene;
}

Camera makeLibreCamera(int imageWidth, int imageHeight)
{
    // Cámara alternativa para la escena "libre":
    // - Posición desplazada: arriba/derecha para dar perspectiva.
    // - FOV 50°: más “tele” (menos distorsión, más compresión de planos).
    double aspectRatio = aspect(imageWidth, imageHeight);
    Vec3 lookFrom(2.2, 1.4, 3.4);
    Vec3 lookAt(-0.2, -0.4, -4.0);
    Vec3 vUp(0.0, 1.0, 0.0);
    double vfov = 50.0;
    return Camera(lookFrom, lookAt, vUp, vfov, aspectRatio);
}

// Consigna (Escena A - Libre): al menos 3 objetos con materiales distintos y
// un plano o fondo visible. Se ajusta `scene.maxDepth`.
Scene makeLibreScene(int maxDepth)
{
    Scene scene;
    // Mismo fondo celeste y control de rebotes.
    scene.backgroundColour = Vec3(0.7, 0.8, 1.0);
    scene.maxDepth = maxDepth;

    // ───────────────────────────────── Materiales ─────────────────────────────────
    Material diffuseAlbedo(MaterialType::Diffuse, Vec3(0.78, 0.25, 0.2));
    diffuseAlbedo.ka = 0.12;
    diffuseAlbedo.kd = 0.9;
    diffuseAlbedo.ks = 0.06;
    diffuseAlbedo.specularColour = Vec3(1.0);
    diffuseAlbedo.shininess = 24.0;

    Material accentGreen(MaterialType::Diffuse, Vec3(0.25, 0.65, 0.38));
    accentGreen.ka = 0.1;
    accentGreen.kd = 0.9;
    accentGreen.ks = 0.04;
    accentGreen.specularColour = Vec3(1.0);
    accentGreen.shininess = 20.0;

    // Metal dorado (reflexivo) con algo más de rugosidad (fuzz=0.1).
    // Sugerencia: subí fuzz a 0.2–0.3 para un metal más “cepillado”.
    Material metalGold(MaterialType::Metal, Vec3(0.94, 0.82, 0.58), 0.1, 1.0, 96.0);
    metalGold.ka = 0.05;
    metalGold.kd = 0.15;
    metalGold.ks = 1.0;
    metalGold.specularColour = Vec3(0.95, 0.82, 0.45);

    // Vidrio con índice 1.5 (refracción tipo "burbuja/vaso").
    // Para vidrio coloreado: setear `absorption = Vec3(r,g,b)` y `absorptionDistance`>0.
    Material dielectricGlass(MaterialType::Dielectric, Vec3(1.0, 1.0, 1.0), 0.0, 1.5, 96.0);
    dielectricGlass.ka = 0.02;
    dielectricGlass.kd = 0.05;
    dielectricGlass.ks = 1.0;
    dielectricGlass.specularColour = Vec3(0.98, 0.99, 1.0);
    dielectricGlass.absorption = Vec3(0.0);
    dielectricGlass.absorptionDistance = 0.0;

    Material groundMat(MaterialType::Diffuse, Vec3(0.9, 0.88, 0.85));
    groundMat.ka = 0.12;
    groundMat.kd = 0.88;
    groundMat.ks = 0.04;
    groundMat.specularColour = Vec3(1.0);
    groundMat.shininess = 10.0;

    Material backWallMat(MaterialType::Diffuse, Vec3(0.76, 0.8, 0.86));
    backWallMat.ka = 0.1;
    backWallMat.kd = 0.9;
    backWallMat.ks = 0.03;
    backWallMat.specularColour = Vec3(1.0);
    backWallMat.shininess = 14.0;

    // ───────────────────────────────── Geometría ─────────────────────────────────
    // Fondo visible: piso (Y = -1) y pared posterior (Z = -7).
    scene.addObject(std::make_shared<Plane>(Vec3(0, -1.0, 0), Vec3(0, 1, 0), groundMat));
    scene.addObject(std::make_shared<Plane>(Vec3(0, 0, -7.0), Vec3(0, 0, 1), backWallMat));

    // Esferas: posiciones pensadas para que se vean separadas y con interacción de luces.
    //  - Izquierda (difusa, pequeña): un poco baja y cercana.
    scene.addObject(std::make_shared<Sphere>(Vec3(-0.85, -0.6, -2.6), 0.4, diffuseAlbedo));
    //  - Centro (vidrio): tamaño medio, más al fondo.
    scene.addObject(std::make_shared<Sphere>(Vec3(0.35, -0.35, -3.8), 0.7, dielectricGlass));
    //  - Derecha (metal dorado): más grande, a media altura.
    scene.addObject(std::make_shared<Sphere>(Vec3(1.75, -0.2, -3.3), 0.85, metalGold));

    // Triángulo (tres puntos en el espacio). Sirve para probar intersección con triángulos.
    // El orden de los vértices define la orientación de la normal (regla de la mano derecha sobre (v1-v0)x(v2-v0)).
    scene.addObject(std::make_shared<Triangle>(Vec3(-2.7, -1.0, -5.2),
                                               Vec3(-1.0, -1.0, -3.0),
                                               Vec3(-1.9, 1.3, -4.3), accentGreen));

    // ───────────────────────────────── Iluminación ─────────────────────────────────
    // Tres luces puntuales con distintos colores e intensidades para generar contraste.
    // Nota: la atenuación cuadrática se aplica en `Scene::traceRay`, por lo que
    // acercar la luz o aumentar su intensidad produce mayor contribución por luz.
    scene.addLight(Light(Vec3(-4.5, 4.5, -3.0), Vec3(1.0, 0.96, 0.92)));
    scene.addLight(Light(Vec3(4.0, 3.0, 1.0), Vec3(0.35, 0.4, 0.5)));
    scene.addLight(Light(Vec3(0.0, 6.0, -8.0), Vec3(0.45, 0.45, 0.45)));

    return scene;
}
