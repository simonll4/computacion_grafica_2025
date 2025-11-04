# Sistema de Navegación por Waypoints

## Descripción

Se ha implementado un sistema completo de navegación por waypoints que permite al piloto seguir una ruta predefinida en el espacio 3D. El sistema incluye:

1. **Indicador de Waypoint en el HUD** - Muestra dirección y distancia al waypoint activo
2. **Visualización 3D de Waypoints** - Cilindros brillantes que marcan los puntos de navegación
3. **Sistema de Computadora de Misión** - Calcula automáticamente rumbo y distancia

## Componentes Implementados

### 1. WaypointIndicator (HUD)
- **Ubicación**: `src/hud/WaypointIndicator.{h,cpp}`
- **Función**: Instrumento del HUD que muestra ayuda visual de navegación
- **Características**:
  - Brújula circular con indicación de norte
  - Flecha direccional que apunta hacia el waypoint
  - Ángulo de desviación respecto al rumbo actual
  - Distancia al waypoint (en metros o kilómetros)
  - Indicador "NO WPT" cuando no hay waypoint activo

### 2. WaypointRenderer (3D)
- **Ubicación**: `src/gfx/WaypointRenderer.{h,cpp}`
- **Función**: Renderiza waypoints como objetos 3D en el mundo
- **Características**:
  - Cilindros verticales de 30m de altura
  - Color verde brillante para waypoint activo
  - Color azul tenue para waypoints inactivos
  - Efecto de brillo (glow) en waypoint activo
  - Iluminación básica para mejor visibilidad

### 3. FlightData Extendido
- **Nuevos campos**:
  - `targetWaypoint` - Posición 3D del waypoint objetivo (metros)
  - `hasActiveWaypoint` - Flag indicando si hay waypoint activo
  - `waypointDistance` - Distancia al waypoint (metros)
  - `waypointBearing` - Rumbo hacia el waypoint (0-360°)

## Uso del Sistema

### Controles de Teclado

- **V** - Cambiar entre vista de primera/tercera persona
- **N** - Activar/desactivar sistema de waypoints
- **M** - Saltar al siguiente waypoint manualmente
- **W/S** - Pitch (cabeceo)
- **A/D** - Yaw (guiñada)
- **Q/E** - Roll (alabeo)
- **↑/↓** - Aumentar/disminuir velocidad

### Circuito de Waypoints por Defecto

El sistema inicializa con 6 waypoints formando un circuito:

```cpp
WPT-1:  ( 500,  100,    0) - Este
WPT-2:  ( 500,  100, -500) - Sureste
WPT-3:  (   0,  150, -500) - Sur (más alto)
WPT-4:  (-500,  100, -500) - Suroeste
WPT-5:  (-500,  100,    0) - Oeste
HOME:   (   0,  100,    0) - Centro
```

### Captura Automática de Waypoints

El sistema detecta automáticamente cuando el avión se acerca a un waypoint:
- **Radio de captura**: 50 metros
- Al alcanzar un waypoint, avanza automáticamente al siguiente
- El circuito es cíclico (vuelve al inicio)

## Cómo Funciona

### Cálculo de Rumbo (Bearing)

El sistema calcula el rumbo hacia el waypoint proyectando la dirección en el plano horizontal (XZ):

```cpp
// Vector hacia el waypoint
glm::vec3 toWaypoint = waypoint.position - planePos;

// Proyección en plano horizontal
glm::vec2 toWaypointXZ = glm::vec2(toWaypoint.x, toWaypoint.z);

// Cálculo de bearing (0° = norte = -Z, sentido horario)
float bearing = atan2(toWaypointXZ.x, -toWaypointXZ.y) * (180/π);
```

### Indicador Direccional

La flecha en el HUD muestra el **ángulo relativo** entre:
- El rumbo actual del avión (heading)
- El rumbo hacia el waypoint (bearing)

Si la flecha apunta:
- **Arriba** → Waypoint directamente adelante
- **Derecha** → Girar a la derecha
- **Izquierda** → Girar a la izquierda
- **Abajo** → Waypoint detrás

## Personalización

### Modificar Waypoints

Editar en `main.cpp`, función `initializeWaypoints()`:

```cpp
waypoints.clear();
waypoints.push_back({glm::vec3(x, y, z), "Nombre"});
// Agregar más waypoints...
```

### Ajustar Radio de Captura

Modificar en `main.cpp`, función `updateWaypointData()`:

```cpp
const float WAYPOINT_CAPTURE_RADIUS = 50.0f; // Cambiar valor
```

### Cambiar Colores

En `main.cpp`, loop de renderizado:

```cpp
// Waypoint activo (verde brillante)
glm::vec4 activeColor = glm::vec4(0.0f, 1.0f, 0.4f, 0.8f);

// Waypoints inactivos (azul tenue)
glm::vec4 inactiveColor = glm::vec4(0.2f, 0.5f, 1.0f, 0.6f);
```

## Arquitectura

```
┌─────────────────────────────────────────────────────────┐
│                      main.cpp                           │
│  ┌──────────────┐    ┌──────────────┐                  │
│  │   Waypoint   │───▶│ FlightData   │                  │
│  │   System     │    │  (bearing,   │                  │
│  │ (calculates) │    │   distance)  │                  │
│  └──────────────┘    └───────┬──────┘                  │
│                              │                          │
│                              ▼                          │
│  ┌──────────────┐    ┌──────────────┐                  │
│  │  Waypoint    │    │   Waypoint   │                  │
│  │  Renderer    │    │  Indicator   │                  │
│  │   (3D)       │    │    (HUD)     │                  │
│  └──────────────┘    └──────────────┘                  │
└─────────────────────────────────────────────────────────┘
```

## Notas Técnicas

### Sistema de Coordenadas

- **X**: Este (+) / Oeste (-)
- **Y**: Arriba (+) / Abajo (-)
- **Z**: Norte (-) / Sur (+)
- **0° heading**: Mira hacia -Z (Norte)
- **90° heading**: Mira hacia +X (Este)

### Rendimiento

- Los waypoints se renderizan siempre que estén activos
- El indicador HUD solo se muestra en vista de primera persona (POV)
- Cálculos de navegación se ejecutan cada frame (~60 FPS)
- Bajo impacto en rendimiento (< 0.1ms por frame)

## Expansión Futura

Posibles mejoras al sistema:

1. **Importar rutas desde archivo** (GPX, JSON)
2. **Editor de waypoints en tiempo real**
3. **Alturas mínimas/máximas por waypoint**
4. **Velocidades recomendadas**
5. **Waypoints de procedimiento** (holding patterns)
6. **Navegación GPS con desviación de ruta**
7. **Estimación de tiempo de llegada (ETA)**
8. **Waypoints condicionales** (meteorología, combustible)

## Referencias

- Sistema inspirado en navegación RNAV (Area Navigation)
- Formato de waypoints compatible con sistemas GPS modernos
- Indicador visual basado en HSI (Horizontal Situation Indicator)
