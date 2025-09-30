# Flight Simulator HUD - OpenGL

Un simulador de vuelo completo con HUD (Head-Up Display) usando OpenGL 3.3 Core Profile. Combina un skybox inmersivo con instrumentos de vuelo realistas que responden al movimiento de la cámara.

## Características

### Sistema de Vuelo
- **Simulación de vuelo realista** - Datos de vuelo calculados desde movimiento de cámara
- **Física básica** - Simulación de sustentación, gravedad y pérdida
- **Controles intuitivos** - WASD para movimiento, QE para altitud, mouse para orientación

### HUD (Head-Up Display)
- **5 Instrumentos principales** - Attitude, Airspeed, Altimeter, Heading, Vertical Speed
- **3 Layouts configurables** - Classic, Modern, Minimal (teclas 1, 2, 3)
- **Renderizado 2D optimizado** - Sistema de batching para máximo rendimiento
- **Colores realistas** - Verde HUD, amarillo para alertas, rojo para peligros

### Gráficos
- **OpenGL 3.3 Core Profile** - Compatible con hardware moderno y legacy
- **Skybox inmersivo** - Soporte múltiples formatos de atlas cubemap
- **Arquitectura modular** - Separación clara entre 3D y 2D rendering
- **Blending avanzado** - Transparencias correctas para overlay HUD

## Estructura del Proyecto

```
HUD/
├── src/
│   ├── main.cpp              # Aplicación principal del simulador
│   ├── gfx/                  # Sistema gráfico
│   │   ├── GLCheck.h         # Utilidades de error OpenGL
│   │   ├── Shader.h/.cpp     # Clase shader modular
│   │   ├── Renderer2D.h/.cpp # Renderizador 2D para HUD
│   │   ├── TextureCube.h/.cpp # Manejo de texturas cubemap
│   │   └── SkyboxRenderer.h/.cpp # Renderizador de skybox
│   ├── hud/                  # Sistema HUD
│   │   ├── FlightHUD.h/.cpp  # Coordinador principal del HUD
│   │   ├── AttitudeIndicator.h/.cpp # Indicador de actitud
│   │   ├── AirspeedIndicator.h/.cpp # Indicador de velocidad
│   │   ├── Altimeter.h/.cpp  # Altímetro
│   │   ├── HeadingIndicator.h/.cpp # Indicador de rumbo
│   │   └── VerticalSpeedIndicator.h/.cpp # Velocidad vertical
│   ├── flight/               # Sistema de vuelo
│   │   └── FlightData.h/.cpp # Datos y física de vuelo
│   └── util/                 # Utilidades
│       └── ImageAtlas.h/.cpp # Procesamiento de atlas
├── shaders/
│   ├── skybox.vert/.frag    # Shaders del skybox
│   └── hud.vert/.frag       # Shaders del HUD 2D
├── Cubemap/                 # Atlas de texturas del cielo
└── include/                 # Headers externos (GLAD, stb_image)
```

## Compilación

```bash
make clean
make build
```

## Ejecución

```bash
make run
# o directamente:
./build/Skybox-Demo
```

## Controles

### Vuelo
- **W/S** - Acelerar/Desacelerar (pitch hacia adelante/atrás)
- **A/D** - Alabeo izquierda/derecha (roll)
- **Q/E** - Subir/Bajar altitud
- **Mouse** - Control de pitch y yaw (cabeceo y guiñada)
- **ESC** - Salir del simulador

### HUD
- **1** - Layout Clásico (instrumentos grandes y separados)
- **2** - Layout Moderno (compacto y eficiente)
- **3** - Layout Mínimo (solo elementos esenciales)

## Detalles Técnicos

### Atlas de Cubemap

El sistema detecta automáticamente el formato del atlas:
- **Horizontal Cross (4x3)** - Layout estándar con cruz horizontal
- **Vertical Cross (3x4)** - Layout con cruz vertical
- **Row (6x1)** - 6 caras en fila horizontal
- **Column (1x6)** - 6 caras en columna vertical
- **Single 512x512** - Layout especial para texturas cuadradas

### Orden de Caras OpenGL

Las caras se mapean en el orden estándar de OpenGL:
1. **+X** (Right)
2. **-X** (Left)  
3. **+Y** (Top)
4. **-Y** (Bottom)
5. **+Z** (Front)
6. **-Z** (Back)

### Instrumentos del HUD

#### 1. Attitude Indicator (Indicador de Actitud)
- **Horizonte artificial** con cielo azul y tierra marrón
- **Escala de pitch** cada 10° con líneas de referencia
- **Escala de roll** de -60° a +60° en arco superior
- **Símbolo de aeronave** fijo en el centro

#### 2. Airspeed Indicator (Indicador de Velocidad)
- **Escala vertical** de 0 a 500 nudos
- **Marcas principales** cada 20 nudos
- **Caja digital** con velocidad actual
- **Colores de alerta** para velocidades críticas

#### 3. Altimeter (Altímetro)
- **Escala vertical** de -1000 a 50000 pies
- **Marcas principales** cada 500 pies
- **Caja digital** con altitud actual
- **Referencia de nivel del mar**

#### 4. Heading Indicator (Indicador de Rumbo)
- **Escala horizontal** de 0° a 360°
- **Marcas principales** cada 30° (puntos cardinales)
- **Caja digital** con rumbo actual
- **Puntero central** fijo

#### 5. Vertical Speed Indicator (Velocidad Vertical)
- **Escala no lineal** de -6000 a +6000 pies/min
- **Línea de referencia** en cero (vuelo nivelado)
- **Puntero triangular** con colores de estado
- **Compresión de escala** para valores extremos

### Shaders

#### Skybox (3D)
- **Vertex Shader**: Transforma posiciones del cubo, elimina traslación
- **Fragment Shader**: Samplea cubemap con dirección normalizada

#### HUD (2D)
- **Vertex Shader**: Proyección ortográfica para overlay 2D
- **Fragment Shader**: Renderizado de colores sólidos con alpha blending

### Optimizaciones

#### Renderizado 3D
- **Depth Function**: `GL_LEQUAL` para skybox en el fondo
- **View Matrix**: Sin traslación para skybox centrado
- **Culling**: Face culling optimizado para geometría

#### Renderizado 2D
- **Batching**: Agrupación de primitivas para reducir draw calls
- **Alpha Blending**: Transparencias correctas para overlay
- **Viewport**: Coordenadas de pantalla para UI precisa

## Dependencias

- **GLFW** - Manejo de ventanas y input
- **GLAD** - Carga de funciones OpenGL
- **GLM** - Matemáticas 3D
- **stb_image** - Carga de imágenes

## Extensiones Futuras

### Sistema de Vuelo
- **Modelo aerodinámico avanzado** - Sustentación, resistencia, efectos de viento
- **Sistemas de aeronave** - Motor, combustible, sistemas eléctricos
- **Navegación** - Waypoints, rutas de vuelo, ILS
- **Condiciones meteorológicas** - Viento, turbulencias, visibilidad

### HUD Avanzado
- **Renderizado de texto** - Números y etiquetas reales
- **Instrumentos adicionales** - Turn coordinator, engine parameters
- **Alertas y warnings** - Sistema de avisos visuales y sonoros
- **Modos de vuelo** - Autopilot, approach modes

### Gráficos
- **HDR Pipeline** - Soporte para cubemaps HDR (EXR/HDR)
- **IBL (Image-Based Lighting)** - Iluminación realista basada en entorno
- **Terrain rendering** - Terreno 3D con heightmaps
- **Weather effects** - Nubes, lluvia, niebla

### Interfaz
- **Menú de configuración** - Ajustes de sensibilidad, layouts
- **Múltiples aeronaves** - Diferentes tipos de avión
- **Replay system** - Grabación y reproducción de vuelos
- **Multiplayer** - Vuelo compartido en red

## Arquitectura del Sistema

### Flujo de Renderizado
1. **Update Phase**: Actualización de datos de vuelo desde input
2. **3D Rendering**: Skybox con depth testing normal
3. **2D Overlay**: HUD con alpha blending sobre 3D
4. **Present**: Swap buffers y poll events

### Separación de Responsabilidades
- **main.cpp**: Loop principal y coordinación
- **flight/**: Lógica de simulación de vuelo
- **gfx/**: Sistemas de renderizado (3D y 2D)
- **hud/**: Instrumentos y layout del HUD

### Patrón de Diseño
- **Component System**: Cada instrumento es independiente
- **Observer Pattern**: HUD responde a cambios en FlightData
- **Strategy Pattern**: Diferentes layouts intercambiables
- **RAII**: Gestión automática de recursos OpenGL

## Notas de Implementación

Este simulador combina técnicas avanzadas de OpenGL:

1. **Dual Rendering Pipeline** - 3D para mundo, 2D para HUD
2. **Real-time Flight Dynamics** - Cálculo de parámetros desde cámara
3. **Modular Architecture** - Fácil extensión y mantenimiento
4. **Performance Optimized** - Batching 2D y minimal state changes
5. **Aviation Accuracy** - Instrumentos basados en estándares reales

La implementación está inspirada en sistemas HUD reales de aviación, adaptada para funcionar como simulador educativo y demostración técnica de OpenGL moderno.
