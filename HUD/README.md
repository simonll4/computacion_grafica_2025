# Flight Simulator HUD - OpenGL 3.3

Sistema profesional de simulador de vuelo con HUD (Heads-Up Display) y terreno realista.

## 🎯 Características Principales

- **HUD con Altímetro de 7 Segmentos**: Display profesional estilo aviación
- **Terreno Realista**: Texturizado triplanar 4K con detail mapping
- **Skybox**: Cielo envolvente con múltiples cubemaps
- **Cámara FPS**: Control libre con mouse y teclado
- **Física Básica**: Simulación de vuelo con colisión de terreno
- **Optimizado**: OpenGL 3.3 Core Profile, 60+ FPS

## 📁 Estructura del Proyecto

```
HUD/
├── src/
│   ├── main.cpp              # Punto de entrada, loop principal
│   ├── flight/
│   │   ├── FlightData.h/cpp  # Datos de vuelo (altitud, velocidad, etc.)
│   ├── gfx/
│   │   ├── Shader.h/cpp          # Sistema de shaders
│   │   ├── Renderer2D.h/cpp      # Renderizador 2D para HUD
│   │   ├── TerrainRenderer.h/cpp # Terreno con triplanar mapping
│   │   ├── TerrainMesh.h/cpp     # Generación de grid NxN
│   │   ├── SkyboxRenderer.h/cpp  # Skybox envolvente
│   │   ├── TextureCube.h/cpp     # Cubemap loader
│   │   └── SimpleCube.h/cpp      # Cubos de referencia
│   ├── hud/
│   │   ├── FlightHUD.h/cpp       # Sistema de HUD
│   │   └── Altimeter.h/cpp       # Altímetro de 7 segmentos
│   └── util/
│       └── ImageAtlas.h/cpp      # Carga de atlas de texturas
├── shaders/
│   ├── terrain.vert/frag     # Shaders del terreno
│   ├── skybox.vert/frag      # Shaders del skybox
│   ├── hud.vert/frag         # Shaders del HUD 2D
│   └── cube.vert/frag        # Shaders de cubos de referencia
├── Cubemap/                  # Atlas de cubemaps para skybox
├── forrest_ground_01_4k.blend/  # Texturas del terreno (4K)
└── Makefile                  # Sistema de build
```

## 🚀 Compilación y Ejecución

### Requisitos

- **Compilador**: GCC/G++ con soporte C++17
- **Librerías**:
  - GLFW3 (windowing)
  - OpenGL 3.3+
  - GLM (matemáticas)
  - STB Image (incluida)

### Build

```bash
make build              # Compilar
./build/Skybox-Demo    # Ejecutar
make clean             # Limpiar
```

## 🎮 Controles

### Movimiento
- **W/A/S/D**: Movimiento horizontal
- **Q/E**: Subir/Bajar (con límite en el piso)
- **Mouse**: Rotar cámara (tipo FPS)

### HUD
- **1**: Layout clásico
- **2**: Layout moderno
- **3**: Layout minimal

### General
- **ESC**: Salir

## 🏗️ Arquitectura Técnica

### Terreno
- **Grid**: 128x128 vértices (~98k índices)
- **Triplanar Mapping**: Proyección en 3 ejes, sin estiramientos
- **Detail Texture**: Capa de detalle para romper repetición
- **Floating Origin**: Grid anclado a cámara (terreno "infinito")
- **Texturas**: 4K con mipmaps automáticos
- **Niebla**: Exponencial configurable

### Altímetro
- **Display de 7 Segmentos**: Estilo aviación profesional
- **Cinta Vertical**: Scroll continuo cada 100 pies
- **Culling Inteligente**: Solo dibuja números visibles
- **Piso de Referencia**: Y=1.8m = 0 pies de altitud
- **Conversión**: 1 metro = 3.28084 pies

### Cámara
- **Tipo**: FPS con ángulos de Euler (yaw/pitch)
- **Límites**: Pitch ±89° (evita gimbal lock)
- **Colisión**: No puede bajar de Y=1.8m (altura de ojos)
- **Velocidad**: 10 m/s (configurable en kCameraSpeed)
- **Mouse**: Sensibilidad 0.1 (configurable en kMouseSensitivity)

## ⚙️ Configuración

### Constantes en main.cpp

```cpp
// Configuración de ventana
kWindowWidth = 1280
kWindowHeight = 720

// Configuración de cámara
kGroundLevel = 1.8f        // Altura mínima (metros)
kCameraSpeed = 10.0f       // Velocidad de movimiento (m/s)
kMouseSensitivity = 0.1f   // Sensibilidad de rotación
```

### Parámetros del Terreno

```cpp
terrainParams.groundY = 0.0f           // Nivel del piso
terrainParams.tileScaleMacro = 0.05f   // Escala textura principal
terrainParams.tileScaleDetail = 0.4f   // Escala textura de detalle
terrainParams.detailStrength = 0.3f    // Mezcla de detalle (0-1)
terrainParams.fogDensity = 0.0f        // Niebla (0 = deshabilitada)
```

## 📊 Rendimiento

- **Target**: 60+ FPS en hardware moderno
- **Vértices del terreno**: 16,641
- **Índices**: 98,304
- **Texturas**: 4096x4096 (albedo, roughness)
- **Draw calls**: ~6 por frame (skybox, terreno, 4 cubos, HUD)

## 🎨 Assets

### Texturas del Terreno
- **Fuente**: Poly Haven / ambientCG
- **Resolución**: 4K (4096x4096)
- **Albedo**: `forrest_ground_01_diff_4k.jpg` (sRGB)
- **Roughness**: `forrest_ground_01_rough_4k.jpg` (Linear)

### Skybox
- **25 variantes** de cielo incluidas
- **Formato**: Atlas 2048x1536 (6 caras de 512x512)
- **Layout**: Detección automática

## 🔧 Detalles de Implementación

### Triplanar Mapping (terrain.frag)
```glsl
// Pesos de proyección con suavizado
vec3 w = triplanarWeights(normal);  // pow(4) para eliminar seams

// Sample en 3 ejes
vec3 aX = texture(uAlbedo, pos.zy * scale).rgb;
vec3 aY = texture(uAlbedo, pos.xz * scale).rgb;
vec3 aZ = texture(uAlbedo, pos.xy * scale).rgb;

// Mezcla con pesos
vec3 albedo = aX*w.x + aY*w.y + aZ*w.z;
```

### Floating Origin
```cpp
// Snap de cámara cada 32 metros
const float snapStep = 32.0f;
glm::vec2 snap = floor(cameraPos.xz / snapStep) * snapStep;
gridOffset = vec3(snap.x, groundY, snap.y);
```

### Altímetro 7-Segmentos
```cpp
// Segmentos: a b c d e f g
//     aaa
//    f   b
//     ggg
//    e   c
//     ddd

bool segments[7] = {...};  // Configuración por dígito
drawRect(...);             // Cada segmento = rectángulo
```

## 🐛 Debug

Para habilitar logs de debug:
1. Descomentar líneas `DEBUG` en `Altimeter.cpp`
2. Descomentar líneas `DEBUG` en `FlightData.cpp`
3. Recompilar: `make build`

## 📚 Referencias

- **Triplanar Mapping**: https://bgolus.medium.com/normal-mapping-for-a-triplanar-shader-10bf39dca05a
- **Floating Origin**: https://blog.unity.com/technology/floating-origin
- **7-Segment Display**: Estándar de aviación civil

## 📄 Licencia

Proyecto educativo - Computación Gráfica 2025

## ✨ Créditos

- **Texturas**: Poly Haven (CC0)
- **Skybox**: Generado con herramientas de CG
- **Implementación**: Trabajo académico original
