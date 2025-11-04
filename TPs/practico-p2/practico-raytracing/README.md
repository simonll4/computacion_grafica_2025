# Ray Tracer - Práctico 2
## Computación Gráfica

### Descripción
Implementación de un ray tracer básico con iluminación local de Phong, sombras duras y reflexiones especulares. El proyecto incluye renderizado de esferas y cubos (AABB) con soporte para múltiples luces y materiales.

### Características Implementadas

#### ✅ Geometría
- **Esfera**: Intersección analítica mediante ecuación cuadrática
- **Cubo (AABB)**: Intersección usando método de slabs
- Cálculo correcto de normales para ambas primitivas

#### ✅ Iluminación (Modelo de Phong)
- **Componente Ambiental**: `I_amb = I_a * k_a`
- **Componente Difusa**: Lambert con `I_diff = f_att * I_p * k_d * max(0, n·s)`
- **Componente Especular**: Phong con `I_spec = f_att * I_p * k_s * max(0, v·r)^n`
- **Atenuación por distancia**: `f_att = 1 / (c1 + c2*d + c3*d²)`

#### ✅ Características Avanzadas
- **Sombras duras**: Rayos de sombra para oclusiones directas
- **Múltiples luces**: Suma de contribuciones por canal RGB
- **Reflexiones especulares**: Recursión limitada (profundidad máxima configurable)
- **Paralelización**: OpenMP para acelerar el renderizado

### Estructura del Proyecto

```
TPs/practico-p2/practico-raytracing/
├── include/
│   ├── raytracer/              # Headers públicos del núcleo
│   │   ├── Ray.h
│   │   ├── Camera.h
│   │   ├── Material.h
│   │   ├── HitRecord.h
│   │   ├── Object.h
│   │   ├── Sphere.h
│   │   ├── AABB.h
│   │   ├── Light.h
│   │   ├── Scene.h
│   │   ├── RayTracer.h
│   │   └── ImageWriter.h
│   └── scenes/
│       └── Scenes.h            # Declaraciones + registro de escenas
├── src/
│   ├── main.cpp                # Programa principal
│   ├── scenes/Scenes.cpp       # Implementación de escenas de prueba
│   └── raytracer/              # Implementaciones del núcleo
│       ├── Camera.cpp
│       ├── AABB.cpp
│       ├── Sphere.cpp
│       ├── Scene.cpp
│       ├── RayTracer.cpp
│       └── ImageWriter.cpp
├── Makefile                    # Build del proyecto
└── README.md                   # Esta documentación
```

### Compilación y Ejecución

#### Requisitos
- **Compilador**: g++ con soporte para C++17
- **Librerías**: GLM (OpenGL Mathematics)
- **OpenMP**: Para paralelización

#### Instalación de dependencias (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install g++ libglm-dev libomp-dev
```

#### Compilar
```bash
make
```

#### Ejecutar

**Opción 1: Menú interactivo**
```bash
make run
# O directamente:
./build/RayTracer
```

**Opción 2: Escena específica**
```bash
./build/RayTracer 1   # Escena principal
./build/RayTracer 2   # Test de sombras
./build/RayTracer 3   # Test de materiales
./build/RayTracer 4   # Test de atenuación
./build/RayTracer 5   # Test de múltiples luces
```

**Opción 3: Todas las escenas**
Usar el menú e ingresar 0, o ejecutar `./run.sh all`.

**Opción 4: Script rápido**
```bash
./run.sh       # Con menú
./run.sh all   # Todas las escenas
```

Esto generará archivos de imagen en la carpeta `results/`:
- `results/*.ppm`: Formato PPM (portable, texto)
- `results/*.bmp`: Formato BMP (más compatible)

#### Limpiar archivos generados
```bash
make clean              # Limpia binarios
make clean-results      # Limpia imágenes en results/
```

### Escenas de Prueba

El proyecto incluye varias escenas de prueba en `src/scenes/Scenes.{h,cpp}`:

1. **Escena Principal** (`createPrincipalScene`): 
   - Esfera roja y cubo azul con esfera verde pequeña
   - Tres luces de diferentes colores
   - Demuestra sombras y múltiples fuentes de luz

2. **Test de Sombras** (`createShadowTestScene`):
   - Validación de proyección de sombras
   - Una luz fuerte para sombras duras definidas

3. **Test de Materiales** (`createMaterialTestScene`):
   - Cuatro esferas con diferentes valores de shininess
   - Demuestra variación en brillos especulares

4. **Test de Atenuación** (`createAttenuationTestScene`):
   - Esferas a diferentes distancias
   - Validación de caída de intensidad lumínica

5. **Test de Múltiples Luces** (`createMultipleLightsScene`):
   - Tres luces de colores primarios (RGB)
   - Demuestra mezcla aditiva de colores

### Parámetros Configurables

En `main.cpp`:
- `IMAGE_WIDTH`, `IMAGE_HEIGHT`: Resolución de la imagen
- `maxDepth`: Profundidad máxima de recursión para reflexiones

En `Camera`:
- Posición, target, vector up
- Field of view (FOV)
- Aspect ratio

En `Material`:
- Coeficientes ka, kd, ks
- Exponente de Phong (shininess)

En `Light`:
- Posición e intensidad/color
- Coeficientes de atenuación

### Algoritmo de Ray Tracing

1. **Generación de rayos**: Por cada píxel, se genera un rayo desde la cámara
2. **Intersección**: Se busca la intersección más cercana con los objetos
3. **Shading**: Se calcula la iluminación de Phong en el punto de intersección
4. **Sombras**: Se lanzan rayos hacia las luces para detectar oclusiones
5. **Reflexiones**: Opcionalmente, se trazan rayos reflejados recursivamente

### Optimizaciones Implementadas

- **Paralelización con OpenMP**: Los píxeles se procesan en paralelo
- **Early termination**: Se detiene la búsqueda al encontrar la primera intersección válida
- **Epsilon para auto-intersección**: Evita artefactos en sombras y reflexiones

### Validación del Práctico

El ray tracer implementa todos los requisitos del plan especificado:

| Requisito | Estado | Implementación |
|-----------|---------|----------------|
| Esfera | ✅ | `Sphere.cpp` - Intersección analítica |
| Cubo AABB | ✅ | `AABB.cpp` - Método de slabs |
| Iluminación Phong | ✅ | `RayTracer.cpp::computePhongShading` |
| Sombras | ✅ | `Scene.cpp::inShadow` |
| Atenuación | ✅ | `Light.h::getAttenuation` |
| Múltiples luces | ✅ | Loop en `computePhongShading` |
| Reflexiones | ✅ | Recursión en `RayTracer.cpp::trace` |

### Mejoras Futuras Posibles

- Anti-aliasing mediante supersampling
- Más primitivas (planos, triángulos)
- Texturas y mapeo UV
- Refracción y transparencia
- Aceleración con BVH (Bounding Volume Hierarchy)
- Global illumination (path tracing)

### Autor
Práctico 2 - Computación Gráfica 2025
