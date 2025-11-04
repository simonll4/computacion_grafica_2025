# Práctico 2 - Ray Tracer
## Computación Gráfica 2025

### 📦 Contenido de la Entrega

Este directorio contiene la implementación completa del ray tracer desarrollado para el Práctico 2.

### 📁 Estructura del Proyecto

```
practico-raytracing/
├── src/
│   ├── main.cpp              # Programa principal con menú interactivo
│   ├── TestScenes.h          # Definición de escenas de prueba
│   └── raytracer/            # Módulos del ray tracer
│       ├── Ray.h             # Representación de rayos
│       ├── Camera.h          # Sistema de cámara y generación de rayos
│       ├── Material.h        # Propiedades de materiales (Phong)
│       ├── HitRecord.h       # Información de intersecciones
│       ├── Object.h          # Interfaz de objetos geométricos
│       ├── Sphere.h          # Implementación de esferas
│       ├── AABB.h            # Implementación de cubos (AABB)
│       ├── Light.h           # Luces puntuales con atenuación
│       ├── Scene.h           # Contenedor de escena
│       ├── RayTracer.h       # Motor principal de ray tracing
│       └── ImageWriter.h     # Exportación de imágenes (PPM/BMP)
├── docs/                     # Documentación detallada
│   ├── RESUMEN_PROYECTO.md   # Resumen ejecutivo
│   ├── IMPLEMENTACION.md     # Detalles técnicos
│   ├── EJEMPLOS_USO.md       # Guía de uso
│   ├── INICIO_RAPIDO.md      # Quick start
│   └── CHECKLIST_ENTREGA.md  # Verificación de requisitos
├── results/                  # Directorio para imágenes generadas
├── build/                    # Archivos compilados
├── Makefile                  # Sistema de construcción
├── run.sh                    # Script de ejecución rápida
├── README.md                 # Documentación principal
└── ENTREGA.md               # Este archivo

```

### 🚀 Inicio Rápido

#### Compilación
```bash
make
```

#### Ejecución con menú interactivo
```bash
./run.sh
# o
make run
```

#### Compilación y ejecución en un comando
```bash
make && make run
```

### ✅ Requisitos Implementados

- [x] **Geometría**: Esferas y cubos (AABB)
- [x] **Iluminación**: Modelo de Phong completo (ambiente, difuso, especular)
- [x] **Sombras**: Sombras duras con rayos de sombra
- [x] **Atenuación**: Caída de intensidad por distancia
- [x] **Múltiples luces**: Soporte para N luces en la escena
- [x] **Reflexiones**: Rayos reflejados con recursión limitada
- [x] **Optimización**: Paralelización con OpenMP

### 📊 Escenas de Prueba Incluidas

1. **Escena Principal**: Demostración completa de todas las características
2. **Test de Sombras**: Validación de proyección de sombras
3. **Test de Materiales**: Variación de shininess
4. **Test de Atenuación**: Caída de intensidad lumínica
5. **Test de Múltiples Luces**: Mezcla aditiva de colores

### 📸 Salida

Las imágenes generadas se guardan en la carpeta `results/` en dos formatos:
- **BMP**: Formato compatible con la mayoría de visualizadores
- **PPM**: Formato portable de texto

### 📚 Documentación

Para más detalles, consultar:
- **README.md**: Documentación completa del proyecto
- **docs/RESUMEN_PROYECTO.md**: Visión general y decisiones de diseño
- **docs/IMPLEMENTACION.md**: Detalles técnicos de implementación
- **docs/EJEMPLOS_USO.md**: Ejemplos de uso y resultados esperados

### 🔧 Dependencias

- **Compilador**: g++ con soporte C++17
- **GLM**: OpenGL Mathematics (libglm-dev)
- **OpenMP**: Para paralelización (libomp-dev)

### 📝 Notas

- El proyecto está completamente autocontenido
- No requiere OpenGL ni GLFW (a diferencia del template original)
- Todas las dependencias son header-only (GLM) o estándar del sistema
- Compatible con Linux/Ubuntu

### 🎯 Verificación Rápida

Para verificar que todo funciona correctamente:

```bash
# Compilar
make clean && make

# Ejecutar escena de prueba
echo "1" | ./build/RayTracer

# Verificar salida
ls -lh results/
```

Deberías ver archivos `scene_*.bmp` y `scene_*.ppm` en la carpeta `results/`.

---

**Fecha de Entrega**: Octubre 2025  
**Asignatura**: Computación Gráfica
