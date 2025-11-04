# Arquitectura del Raytracer - Documentación Completa

## Índice
1. [Visión General](#visión-general)
2. [Estructura de Directorios](#estructura-de-directorios)
3. [Flujo de Ejecución](#flujo-de-ejecución)
4. [Módulos Principales](#módulos-principales)
5. [Algoritmos Clave](#algoritmos-clave)
6. [Guía de Lectura del Código](#guía-de-lectura-del-código)

---

## Visión General

Este es un **raytracer (trazador de rayos)** implementado en C++17 que genera imágenes fotorrealistas mediante simulación física de luz. El programa traza rayos desde una cámara virtual hacia la escena, calculando intersecciones con objetos y simulando iluminación, reflexiones y refracciones.

### Características Principales
- ✅ **Trazado de rayos recursivo** con profundidad configurable
- ✅ **Antialiasing estocástico** (múltiples muestras por píxel)
- ✅ **Tres tipos de geometría**: esferas, planos, triángulos
- ✅ **Tres tipos de materiales**: difuso (mate), metálico (espejo), dieléctrico (vidrio)
- ✅ **Iluminación Phong** con componentes ambiente, difusa y especular
- ✅ **Sombras** mediante rayos de oclusión
- ✅ **Reflexión especular** con rugosidad opcional
- ✅ **Refracción** con ley de Snell y Fresnel-Schlick
- ✅ **Absorción volumétrica** (Beer-Lambert) para vidrios coloreados

---

## Estructura de Directorios

```
raytracer_class/
├── include/rt/              # Headers (.hpp)
│   ├── core/                # Componentes fundamentales
│   │   ├── vec3.hpp         # Vectores 3D (wrapper de GLM)
│   │   ├── ray.hpp          # Estructura de rayo
│   │   ├── camera.hpp       # Cámara pinhole
│   │   ├── random.hpp       # Generadores RNG
│   │   └── renderer.hpp     # Bucle de renderizado
│   ├── geom/                # Primitivas geométricas
│   │   ├── sphere.hpp       # Esfera (cuadrática)
│   │   ├── plane.hpp        # Plano infinito
│   │   └── triangle.hpp     # Triángulo (Möller-Trumbore)
│   ├── scene/               # Escena e iluminación
│   │   ├── scene.hpp        # Contenedor de objetos/luces
│   │   ├── object.hpp       # Clase base + HitRecord
│   │   ├── material.hpp     # Definición de materiales
│   │   ├── material_shading.hpp  # Funciones de shading
│   │   ├── light.hpp        # Luz puntual
│   │   ├── scene_presets.hpp     # Sistema de presets
│   │   └── scenes.hpp       # API pública de escenas
│   └── io/
│       └── image_ppm.hpp    # Escritura de imágenes PPM
├── src/                     # Implementaciones (.cpp)
│   ├── main.cpp             # Punto de entrada
│   ├── renderer.cpp         # Bucle de render + AA + gamma
│   ├── scene.cpp            # Scene::traceRay (recursivo)
│   ├── material_shading.cpp # Iluminación local + global
│   ├── scene_presets.cpp    # Definición de escenas
│   └── scenes.cpp           # Wrappers de API
├── output/                  # Imágenes generadas (.ppm)
├── Makefile                 # Sistema de build
└── ARQUITECTURA.md          # Este documento
```

---

## Flujo de Ejecución

### 1. Inicialización (`main.cpp`)
```
main()
  ├─> Configura parámetros (resolución, SPP, maxDepth)
  ├─> Crea directorio output/
  ├─> Construye escena base (makeBaseScene)
  ├─> Construye cámara base (makeDefaultCamera)
  ├─> Renderiza escena base
  ├─> Guarda output/base.ppm
  ├─> Construye escena libre (makeLibreScene)
  ├─> Construye cámara libre (makeLibreCamera)
  ├─> Renderiza escena libre
  └─> Guarda output/libre.ppm
```

### 2. Renderizado (`renderer.cpp`)
```
render(scene, camera, settings)
  └─> Para cada píxel (i,j):
      └─> Para cada muestra s (antialiasing):
          ├─> Calcula (u,v) con jitter aleatorio
          ├─> Genera rayo primario: camera.getRay(u,v)
          ├─> Traza rayo: scene.traceRay(ray, maxDepth)
          └─> Acumula color
      ├─> Promedia muestras
      ├─> Aplica corrección gamma (2.2)
      └─> Convierte a RGB 8-bit
```

### 3. Trazado de Rayos (`scene.cpp`)
```
Scene::traceRay(ray, depth)
  ├─> Si depth <= 0: return negro
  ├─> Busca intersección más cercana
  ├─> Si no hay hit: return color de fondo (gradiente)
  └─> Si hay hit:
      ├─> Calcula iluminación local (Phong)
      ├─> Calcula contribución global (reflexión/refracción)
      └─> return local + global
```

### 4. Iluminación (`material_shading.cpp`)
```
computeLocalLighting(scene, rec, ray)
  ├─> Componente ambiente: ka * albedo
  └─> Para cada luz:
      ├─> Calcula dirección y distancia
      ├─> Test de sombras (shadow ray)
      ├─> Si no hay sombra:
      │   ├─> Componente difusa: kd * albedo * (N·L)
      │   ├─> Componente especular: ks * specColour * (R·V)^shininess
      │   └─> Aplica atenuación cuadrática
      └─> Acumula contribución

computeGlobalContribution(scene, rec, ray, depth)
  └─> Según material.type:
      ├─> Diffuse: return negro (sin contribución global)
      ├─> Metal: reflexión especular + rugosidad (fuzz)
      └─> Dielectric: mezcla reflexión/refracción (Fresnel)
```

---

## Módulos Principales

### 📐 Módulo Core (`include/rt/core/`)

#### `vec3.hpp` - Vectores 3D
- **Propósito**: Wrapper sobre `glm::dvec3` para operaciones vectoriales
- **Funciones clave**:
  - `dot(a,b)`: producto punto
  - `cross(a,b)`: producto cruz
  - `normalized(v)`: normalización
  - `reflect(i,n)`: reflexión especular
  - `refract(i,n,eta_i,eta_t,out)`: refracción (Snell)

#### `ray.hpp` - Rayo 3D
- **Estructura**: `origin` + `direction`
- **Método**: `at(t)` evalúa `origin + t*direction`

#### `camera.hpp` - Cámara Pinhole
- **Sistema de coordenadas**:
  - `w`: eje de vista (lookFrom → lookAt)
  - `u`: eje horizontal (derecha)
  - `v`: eje vertical (arriba)
- **Método clave**: `getRay(s,t)` genera rayos primarios

#### `random.hpp` - Generadores Aleatorios
- **Thread-safe**: usa `thread_local` para paralelización
- **Funciones**:
  - `random_double()`: [0,1) uniforme
  - `random_in_unit_sphere()`: punto dentro de esfera (método rechazo)
  - `random_unit_vector()`: vector unitario (coordenadas esféricas)

#### `renderer.hpp/cpp` - Bucle de Renderizado
- **Antialiasing**: jitter estocástico con múltiples muestras
- **Corrección gamma**: `sRGB = linear^(1/2.2)`
- **Progreso**: muestra scanlines restantes en stderr

---

### 🔺 Módulo Geometría (`include/rt/geom/`)

#### `sphere.hpp` - Esfera
- **Intersección**: resolución de cuadrática `||o+td-c||² = R²`
- **Discriminante**: `b² - ac` determina número de raíces
- **Normal**: `(p - center) / radius`

#### `plane.hpp` - Plano Infinito
- **Definición**: punto + normal
- **Intersección**: `t = dot(point-o, n) / dot(d, n)`
- **Caso especial**: rayo paralelo (denominador ≈ 0)

#### `triangle.hpp` - Triángulo
- **Algoritmo**: Möller-Trumbore (intersección directa)
- **Coordenadas baricéntricas**: `u,v` con `u≥0, v≥0, u+v≤1`
- **Normal**: `cross(v1-v0, v2-v0)` (regla mano derecha)

---

### 🎨 Módulo Escena (`include/rt/scene/`)

#### `object.hpp` - Clase Base
- **HitRecord**: almacena información de intersección
  - `point`: punto de impacto
  - `normal`: normal orientada contra el rayo
  - `t`: parámetro del rayo
  - `frontFace`: cara frontal/trasera
  - `material`: material del objeto
- **Método virtual**: `intersect(ray, t_min, t_max, rec)`

#### `material.hpp` - Materiales
- **MaterialType**: `Diffuse`, `Metal`, `Dielectric`
- **Parámetros Phong**:
  - `ka`: coeficiente ambiente
  - `kd`: coeficiente difuso
  - `ks`: coeficiente especular
  - `shininess`: exponente de brillo
- **Parámetros específicos**:
  - Metal: `fuzz` (rugosidad)
  - Dieléctrico: `refractiveIndex`, `absorption`, `absorptionDistance`

#### `scene.hpp/cpp` - Contenedor de Escena
- **Atributos**:
  - `objects`: vector de objetos
  - `lights`: vector de luces
  - `backgroundColour`: color de fondo
  - `maxDepth`: profundidad recursiva máxima
- **Método clave**: `traceRay(ray, depth)` - corazón del raytracer

#### `material_shading.hpp/cpp` - Modelos de Iluminación
- **Iluminación local**: modelo Phong completo
  - Ambiente: `ka * albedo`
  - Difusa: `kd * albedo * max(N·L, 0)`
  - Especular: `ks * specColour * (R·V)^shininess`
  - Sombras: test de oclusión por luz
  - Atenuación: `1 / (1 + k₁d + k₂d²)`
- **Contribución global**:
  - Metal: reflexión con rugosidad opcional
  - Dieléctrico: Fresnel-Schlick + refracción + absorción

#### `scene_presets.hpp/cpp` - Sistema de Configuración
- **Structs de datos**:
  - `SphereSpec`, `PlaneSpec`, `TriangleSpec`: geometría + material
  - `LightSpec`: posición + intensidad
  - `CameraPreset`: eye, target, up, vfov
  - `ScenePreset`: colección completa de objetos/luces
- **Presets definidos**:
  - `kBaseScene`: tres esferas con materiales distintos
  - `kLibreScene`: composición artística con triángulo
  - `kBaseCamera`, `kLibreCamera`: configuraciones de cámara
- **Builders**: `buildScene()`, `buildCamera()` construyen objetos runtime

---

## Algoritmos Clave

### 🔍 Intersección Rayo-Esfera
```
Ecuación: ||o + td - c||² = R²
Expandida: a·t² + 2b·t + c = 0
  donde:
    a = ||d||²
    b = (o-c)·d
    c = ||o-c||² - R²

Discriminante Δ = b² - ac:
  Δ < 0: sin intersección
  Δ = 0: tangente (1 raíz)
  Δ > 0: dos intersecciones

Raíces: t = (-b ± √Δ) / a
Elegir la más cercana en [t_min, t_max]
```

### 🔺 Intersección Rayo-Triángulo (Möller-Trumbore)
```
Rayo: R(t) = o + td
Triángulo: T(u,v) = v₀ + u·e₁ + v·e₂
  donde e₁ = v₁-v₀, e₂ = v₂-v₀

Sistema: o + td = v₀ + u·e₁ + v·e₂
Resuelve para (t,u,v) usando productos cruz

Condiciones:
  - u ≥ 0
  - v ≥ 0
  - u + v ≤ 1
  - t ∈ [t_min, t_max]
```

### 💡 Modelo de Iluminación Phong
```
I = Iₐ + Σ(Iᵈ + Iₛ) * atenuación

Ambiente:  Iₐ = ka * albedo
Difusa:    Iᵈ = kd * albedo * max(N·L, 0)
Especular: Iₛ = ks * specColour * max(R·V, 0)^shininess

donde:
  N = normal de superficie
  L = dirección hacia la luz
  R = reflexión de L sobre N
  V = dirección hacia el observador
```

### 🪞 Reflexión Especular
```
Dirección reflejada: R = I - 2(I·N)N

Con rugosidad (fuzz):
  R' = normalize(R + fuzz * random_in_unit_sphere())
```

### 🔮 Refracción (Ley de Snell)
```
η₁ sin θ₁ = η₂ sin θ₂

Reflexión total interna cuando:
  (η₁/η₂) sin θ₁ > 1

Fresnel-Schlick (reflectancia):
  R(θ) = R₀ + (1-R₀)(1-cos θ)⁵
  donde R₀ = ((η₂-η₁)/(η₂+η₁))²

Color final = R·reflexión + (1-R)·refracción
```

### 🎨 Absorción Volumétrica (Beer-Lambert)
```
Transmitancia = e^(-σ·d)

donde:
  σ = coeficiente de absorción (RGB)
  d = distancia atravesada

Aplicado por canal:
  T = (e^(-σᵣ·d), e^(-σᵍ·d), e^(-σᵦ·d))
```

---

## Guía de Lectura del Código

### Para Entender el Flujo Completo
1. **`main.cpp`** - Punto de entrada, configura y orquesta
2. **`renderer.cpp`** - Bucle de píxeles, antialiasing, gamma
3. **`scene.cpp::traceRay()`** - Recursión de trazado
4. **`material_shading.cpp`** - Cálculos de iluminación

### Para Entender la Geometría
1. **`object.hpp`** - Interfaz base y HitRecord
2. **`sphere.hpp`** - Intersección cuadrática
3. **`plane.hpp`** - Intersección plano-rayo
4. **`triangle.hpp`** - Möller-Trumbore

### Para Entender los Materiales
1. **`material.hpp`** - Definición de tipos y parámetros
2. **`material_shading.cpp::computeLocalLighting()`** - Phong
3. **`material_shading.cpp::computeMetalContribution()`** - Reflexión
4. **`material_shading.cpp::computeDielectricContribution()`** - Refracción

### Para Modificar las Escenas
1. **`scene_presets.cpp`** - Edita `kBaseScene` o `kLibreScene`
2. Modifica materiales, posiciones, luces directamente
3. Recompila con `make`

### Para Agregar Nueva Geometría
1. Crea nuevo header en `include/rt/geom/`
2. Hereda de `Object`
3. Implementa `intersect(ray, t_min, t_max, rec)`
4. Calcula normal y llama `rec.setFaceNormal()`

---

## Parámetros de Tuning

### Calidad de Imagen
- **SPP (samples per pixel)**: 1=rápido/dentado, 4=balanceado, 16+=suave/lento
- **Resolución**: 800x600=rápido, 1920x1080=HD (4x más lento)
- **maxDepth**: 1=sin reflexiones, 5=balanceado, 10+=múltiples rebotes

### Materiales Difusos
- `ka`: 0.05-0.15 (ambiente)
- `kd`: 0.7-1.0 (color principal)
- `ks`: 0.0-0.08 (brillo sutil)
- `shininess`: 8-32 (highlight suave)

### Materiales Metálicos
- `ks`: ≈1.0 (máxima reflexión)
- `kd`: 0.0-0.2 (tinte metálico)
- `fuzz`: 0=espejo perfecto, 0.3=metal cepillado
- `shininess`: 64-128+ (highlight concentrado)

### Materiales Dieléctricos
- `refractiveIndex`: aire=1.0, agua=1.33, vidrio=1.5, diamante=2.42
- `ks`: ≈1.0
- `kd`: muy bajo (0.0-0.1)
- `absorption`: Vec3(0)=transparente, >0=vidrio coloreado
- `absorptionDistance`: distancia de referencia para atenuación

### Cámara
- **FOV alto (80-100°)**: perspectiva amplia, más distorsión
- **FOV bajo (35-60°)**: efecto telefoto, menos distorsión
- **Posición**: mover en Z cambia distancia aparente

### Luces
- **intensity**: RGB lineal (no sRGB)
- **Posición**: más cerca = más brillo (atenuación cuadrática)
- **Múltiples luces**: colores distintos crean contraste

---

## Compilación y Ejecución

```bash
# Compilar
make

# Limpiar y recompilar
make clean && make

# Ejecutar (genera output/base.ppm y output/libre.ppm)
./build/raytracer

# Convertir PPM a PNG (si tienes ImageMagick)
convert output/base.ppm output/base.png
```

---

## Dependencias

- **C++17**: características modernas del lenguaje
- **GLM**: librería matemática para vectores/matrices
- **STL**: contenedores, algoritmos, I/O

---

## Notas de Implementación

### Optimizaciones Aplicadas
- ✅ Reserva de capacidad en vectores (`reserve()`)
- ✅ Forward declarations para reducir dependencias
- ✅ Normalización de direcciones de rayos
- ✅ Bias numérico para evitar auto-intersecciones
- ✅ Thread-local RNG para paralelización futura

### Limitaciones Conocidas
- Sin aceleración espacial (BVH/KD-tree)
- Sin paralelización multi-hilo
- Sin path tracing (solo Phong + reflexión/refracción)
- Formato de salida solo PPM (sin PNG/JPEG directo)

### Posibles Extensiones
- [ ] BVH para escenas grandes
- [ ] Paralelización con OpenMP/TBB
- [ ] Depth of field (desenfoque)
- [ ] Motion blur
- [ ] Texturas procedurales
- [ ] Normal mapping
- [ ] Path tracing completo
- [ ] Denoising

---

**Autor**: Sistema de presets implementado para facilitar configuración  
**Fecha**: Noviembre 2025  
**Versión**: 1.0
