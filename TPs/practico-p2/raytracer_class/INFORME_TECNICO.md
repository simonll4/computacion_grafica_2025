# Informe técnico — Segundo Parcial CGyAV (0494)

Tema: Iluminación global mediante trazado de rayos recursivo (ray-tracing)

---

## 1. Descripción general del proyecto

- Estructura del código
  - Núcleo (`include/rt/core/*`): `vec3` (wrapper de GLM), `ray`, `camera`, RNG y `renderer`.
  - Geometría (`include/rt/geom/*`): `sphere`, `plane`, `triangle` con sus tests de intersección.
  - Escena (`include/rt/scene/*`): `object`/`HitRecord`, `material`, `light`, `scene`, `material_shading`.
  - Presets (`include/rt/scene/scene_presets.hpp`, `src/scene_presets.cpp`): definen “escena base” y “escena libre” + cámaras.
  - Implementaciones (`src/*.cpp`): `renderer.cpp`, `scene.cpp` (trazado recursivo), `material_shading.cpp` (Phong, sombras, reflexión y refracción), `main.cpp` (orquestación) y `scenes.cpp` (wrappers).

- Decisiones de diseño
  - Separación de “iluminación local” (Phong con sombras) y “contribución global” (reflexión/refracción) por material, para mantener `Scene::traceRay` simple y extensible.
  - Uso de GLM únicamente como librería matemática (en línea con las limitaciones de la consigna); el trazador, intersecciones y shading son propios.
  - Control de profundidad recursiva por escena (`Scene::maxDepth`) y parámetro `depth` en `traceRay` para acotar rebotes y evitar bucles.
  - Antialiasing estocástico (múltiples muestras por píxel con jitter) para mejorar bordes y reducir aliasing sin complejizar la arquitectura.
  - Bias numérico consistente (`1e-4`) para rayos de sombra y secundarios, evitando auto-intersecciones (“shadow acne”).
  - Sistema de presets inmutable para reproducibilidad de escenas/cámaras y facilitar ajustes sin tocar el núcleo del raytracer.

- Problemas encontrados y cómo se resolvieron
  - Sombras auto-proyectadas por error numérico: se resolvió agregando un bias constante al origen de rayos de sombra y secundarios.
  - Normales invertidas en refracción: se estandarizó `HitRecord::setFaceNormal` y el intercambio `eta_i/eta_t` según si el rayo entra o sale, corrigiendo Fresnel y reflexión interna total.
  - “Fugas” de energía en metales rugosos: se normaliza la dirección reflejada luego de sumar el término de rugosidad (`fuzz * random_in_unit_sphere`).
  - Saturación de color: se aplica corrección gamma (2.2) al final del pipeline antes de volcar a PPM.

- Cumplimiento de requisitos mínimos (consigna)
  - Rayos primarios desde la cámara: `Camera::getRay(s,t)` y uso en `renderer.cpp`.
  - Intersecciones con objetos simples: `sphere.hpp` (cuadrática) y `triangle.hpp` (Möller–Trumbore); además `plane.hpp`.
  - Iluminación local con luz puntual: Phong con ambiente/difuso/especular y atenuación + test de sombras en `material_shading.cpp`.
  - Iluminación global recursiva: sombras, reflexión (metales con rugosidad) y refracción (dieléctricos con Fresnel-Schlick, reflexión interna y absorción Beer–Lambert).
  - Control de profundidad y color de fondo: `Scene::maxDepth`, parámetro `depth` y `backgroundColour`.
  - Tres materiales distintos: `Diffuse`, `Metal` y `Dielectric` con parámetros específicos (albedo, ks/kd/ka, shininess, fuzz, IOR, absorción).

---

## 2. Croquis de la escena con sistema de referencia

A continuación documento la composición de ambas escenas utilizadas (Base y Libre), con ejes en sistema de mundo (X a la derecha, Y hacia arriba, Z hacia el fondo negativo). Los croquis son aproximados y se basan en las posiciones declaradas en los presets.

- Escena Base (plano + pared + 3 esferas)

  Cámara `kBaseCamera`: eye≈(0.8, 0.9, 1.7), lookAt≈(-0.2, -0.2, -3.5), vfov≈55°

  Top (vista superior, eje X-Z):

  ```
     Z↑ (−)
       
        Pared z=−7                         Luz (0,3,−3)
        ─────────────────────────────────────────●─────
                 Esfera metálica (2,0,−4)
                               ○
       Esfera difusa (−1.5,0,−4.5)        Esfera dieléctrica (0,−0.3,−2.5)
                 ○                                         ○
  Cámara (0.8,0.9,1.7) ● → mira hacia −Z
  X→ (＋)
  ```

  Frontal (vista Y-Z):

  ```
  Y↑
     ○  (metálica)
  ○        (difusa)       ○ (dieléctrica)
  ───────────────────────────────────────── z (pared a −7)
     (piso y=−1)
  ```

- Escena Libre (plano + pared + triángulo + 3 esferas + 3 luces)

  Cámara `kLibreCamera`: eye≈(2.2, 1.4, 3.4), lookAt≈(−0.2, −0.4, −4.0), vfov≈50°

  Top (X-Z):

  ```
     Z↑ (−)

  Triángulo (~z=−4)                Esferas: (-0.85,−0.6,−2.6) difusa
     /\                                                    ○
    /__\                                   (0.35,−0.35,−3.8) dieléctrica ○
  (−3.4,−1,−4) etc.                               (1.75,−0.2,−3.3) metálica   ○

  Luces: (−4.5,4.5,−3), (4,3,1), (0,6,−8)
  Cámara (2.2,1.4,3.4) ● → mira hacia −Z
  X→ (＋)
  ```

  Frontal (Y-Z):

  ```
  Y↑
     ○ (metálica)
        ○ (dieléctrica)
    ○ (difusa)
  ───────────────────────────────────────── z (pared a −7)
     (piso y=−1)
  ```

Notas:
- El plano de piso está en y=−1; la pared posterior en z≈−7 (ambas difusas).
- Los croquis ayudan a justificar luces, sombras y reflejos visibles en las capturas.

---

## 3. Capturas de resultados y explicación

- Cómo generar las imágenes
  - Compilar: `make`
  - Ejecutar: `./build/raytracer` (genera `output/base.ppm` y `output/libre.ppm`)
  - Opcional: convertir a PNG con el script `output/conversor.py` o con ImageMagick.

- Imágenes producidas
  - Base: `output/base.ppm`
  - Libre: `output/libre.ppm`

- Observaciones técnicas sobre los resultados
  - Sombras nítidas (luz puntual): las oclusiones directas se aprecian sobre el piso y la pared.
  - Reflexión metálica: la esfera metálica refleja el entorno (piso/parede(s)) con highlight especular controlado por `shininess`; el parámetro `fuzz` añade un leve desenfoque a la reflexión.
  - Refracción dieléctrica: la esfera de vidrio muestra distorsión del fondo (ley de Snell), mezcla con reflexión según el ángulo (Fresnel-Schlick) y ligera coloración por absorción volumétrica.
  - Iluminación local (Phong): la componente difusa domina en superficies mates; los highlights especulares son más concentrados con `shininess` alto.
  - Corrección gamma: los tonos medios y sombras se ven naturales tras aplicar `γ=2.2` en el postproceso.

---

## 4. Reflexión personal (obligatoria)

Esta sección debe ser confeccionada de manera personal y sin uso de herramientas de IA, tal como exige la consigna. A continuación dejo un esquema de guía para completar a mano:

- ¿Qué partes del código fueron generadas o asistidas mediante herramientas de IA?
  - [Completar]
- ¿Qué mejoras, modificaciones o correcciones fueron necesarias sobre estas partes del código?
  - [Completar]
- ¿Qué conceptos resultaron más difíciles de aplicar y por qué?
  - [Completar]
- ¿Cómo verificó que la reflexión y refracción se comporten correctamente?
  - [Completar]

---

Fin del informe.
