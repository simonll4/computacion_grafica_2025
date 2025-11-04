# Ejemplos de Uso - Ray Tracer

## Inicio Rápido

### Opción 1: Script automático (recomendado)
```bash
./run.sh
```

### Opción 2: Compilación manual
```bash
# Compilar
make

# Ejecutar con menú interactivo
./raytracer

# Ejecutar escena específica
./raytracer 1    # Escena principal
./raytracer 2    # Test de sombras
./raytracer 3    # Test de materiales
./raytracer 4    # Test de atenuación
./raytracer 5    # Test de múltiples luces

# Renderizar todas las escenas automáticamente
./run.sh all
```

## Personalización de Escenas

### Crear una nueva escena

Edita `src/main.cpp` o `src/scenes/Scenes.cpp`:

```cpp
Scene createMyCustomScene() {
    Scene scene;
    
    // 1. Configurar luz ambiental
    scene.setAmbientLight(glm::vec3(0.1f, 0.1f, 0.15f));
    
    // 2. Definir materiales
    Material myMaterial(
        glm::vec3(0.1f),        // ka - Componente ambiental
        glm::vec3(0.8f, 0.3f, 0.2f),  // kd - Difuso (color)
        glm::vec3(0.9f),        // ks - Especular
        64.0f                   // n - Shininess
    );
    
    // 3. Agregar objetos
    // Esfera
    scene.addObject(std::make_shared<Sphere>(
        glm::vec3(0.0f, 0.0f, -5.0f),  // Centro
        1.5f,                           // Radio
        myMaterial
    ));
    
    // Cubo AABB
    scene.addObject(std::make_shared<AABB>(
        glm::vec3(-2.0f, -1.0f, -7.0f),  // Punto mínimo
        glm::vec3(0.0f, 1.0f, -5.0f),    // Punto máximo
        myMaterial
    ));
    
    // O cubo desde centro y tamaño
    scene.addObject(std::make_shared<AABB>(
        glm::vec3(3.0f, 0.0f, -6.0f),  // Centro
        2.0f,                           // Tamaño
        myMaterial
    ));
    
    // 4. Agregar luces
    Light mainLight(
        glm::vec3(0.0f, 5.0f, 0.0f),   // Posición
        glm::vec3(1.0f, 1.0f, 1.0f)    // Color/Intensidad
    );
    
    // Ajustar atenuación (opcional)
    mainLight.constant = 1.0f;
    mainLight.linear = 0.09f;
    mainLight.quadratic = 0.032f;
    
    scene.addLight(mainLight);
    
    return scene;
}
```

### Ajustar la cámara

En `runRayTracer()` de `main.cpp`:

```cpp
Camera camera(
    glm::vec3(0.0f, 2.0f, 5.0f),    // Posición de la cámara
    glm::vec3(0.0f, 0.0f, -5.0f),   // Punto al que mira
    glm::vec3(0.0f, 1.0f, 0.0f),    // Vector up
    45.0f,                           // FOV (field of view)
    float(IMAGE_WIDTH) / float(IMAGE_HEIGHT),  // Aspect ratio
    1.0f                             // Distancia del plano near
);
```

### Modificar resolución

En `main.cpp`:

```cpp
static const int IMAGE_WIDTH = 1920;  // Cambiar aquí
static const int IMAGE_HEIGHT = 1080;

// O para resoluciones comunes:
// 640x480   (VGA)
// 800x600   (SVGA)
// 1280x720  (HD)
// 1920x1080 (Full HD)
// 3840x2160 (4K)
```

**Nota**: A mayor resolución, mayor tiempo de renderizado.

### Ajustar profundidad de reflexiones

En `runRayTracer()`:

```cpp
RayTracer raytracer(IMAGE_WIDTH, IMAGE_HEIGHT, 5);  // Hasta 5 reflexiones
```

Valores recomendados:
- `0`: Sin reflexiones
- `1-2`: Reflexiones básicas (rápido)
- `3-4`: Reflexiones múltiples (óptimo)
- `5+`: Reflexiones profundas (lento)

## Parámetros de Materiales

### Shininess (exponente de Phong)

Controla el tamaño del brillo especular:

```cpp
Material dull(amb, diff, spec, 4.0f);      // Opaco, brillo amplio
Material normal(amb, diff, spec, 32.0f);   // Brillo medio
Material glossy(amb, diff, spec, 128.0f);  // Muy brillante
Material mirror(amb, diff, spec, 512.0f);  // Casi espejo
```

### Coeficientes ka, kd, ks

```cpp
// Material metálico
Material metal(
    glm::vec3(0.05f),       // ka bajo - poca luz ambiental
    glm::vec3(0.3f),        // kd bajo - poca difusión
    glm::vec3(0.9f),        // ks alto - mucho especular
    256.0f
);

// Material plástico
Material plastic(
    glm::vec3(0.1f),        // ka medio
    glm::vec3(0.7f),        // kd alto - mucha difusión
    glm::vec3(0.4f),        // ks medio
    32.0f
);

// Material difuso (matte)
Material matte(
    glm::vec3(0.2f),        // ka alto
    glm::vec3(0.8f),        // kd muy alto
    glm::vec3(0.1f),        // ks muy bajo
    8.0f
);
```

### Colores

Los coeficientes son vectores RGB en [0,1]:

```cpp
glm::vec3 red(1.0f, 0.0f, 0.0f);
glm::vec3 green(0.0f, 1.0f, 0.0f);
glm::vec3 blue(0.0f, 0.0f, 1.0f);
glm::vec3 yellow(1.0f, 1.0f, 0.0f);
glm::vec3 cyan(0.0f, 1.0f, 1.0f);
glm::vec3 magenta(1.0f, 0.0f, 1.0f);
glm::vec3 white(1.0f, 1.0f, 1.0f);
glm::vec3 gray(0.5f, 0.5f, 0.5f);
glm::vec3 orange(1.0f, 0.5f, 0.0f);
```

## Configuración de Luces

### Luz básica

```cpp
Light light(
    glm::vec3(x, y, z),      // Posición
    glm::vec3(r, g, b)       // Color e intensidad
);
scene.addLight(light);
```

### Luz intensa

```cpp
Light bright(pos, glm::vec3(2.0f, 2.0f, 2.0f));  // 2x intensidad
```

### Luz de color

```cpp
Light redLight(pos, glm::vec3(1.0f, 0.0f, 0.0f));    // Roja
Light blueLight(pos, glm::vec3(0.0f, 0.5f, 1.0f));   // Azul cielo
```

### Atenuación personalizada

```cpp
Light light(pos, color);

// Sin atenuación (luz constante)
light.constant = 1.0f;
light.linear = 0.0f;
light.quadratic = 0.0f;

// Atenuación suave (alcance largo)
light.constant = 1.0f;
light.linear = 0.027f;
light.quadratic = 0.0028f;

// Atenuación media (alcance medio)
light.constant = 1.0f;
light.linear = 0.09f;
light.quadratic = 0.032f;

// Atenuación fuerte (alcance corto)
light.constant = 1.0f;
light.linear = 0.22f;
light.quadratic = 0.20f;
```

## Escenarios Comunes

### Escena de estudio (3 luces)

```cpp
// Key light (principal)
Light key(glm::vec3(5, 5, 5), glm::vec3(1.0f));

// Fill light (relleno)
Light fill(glm::vec3(-3, 3, 5), glm::vec3(0.3f));

// Back light (contraluz)
Light back(glm::vec3(0, 3, -5), glm::vec3(0.5f));
```

### Escena nocturna

```cpp
scene.setAmbientLight(glm::vec3(0.02f, 0.02f, 0.05f));  // Azul oscuro

Light moon(glm::vec3(10, 20, 0), glm::vec3(0.3f, 0.3f, 0.4f));
```

### Escena de atardecer

```cpp
scene.setAmbientLight(glm::vec3(0.1f, 0.05f, 0.02f));  // Cálido

Light sun(glm::vec3(20, 10, 20), glm::vec3(1.0f, 0.6f, 0.3f));
```

## Optimización

### Para renders rápidos (pruebas)

```cpp
// Resolución baja
IMAGE_WIDTH = 400;
IMAGE_HEIGHT = 300;

// Sin reflexiones
maxDepth = 0;

// Menos objetos en la escena
```

### Para renders de alta calidad

```cpp
// Resolución alta
IMAGE_WIDTH = 1920;
IMAGE_HEIGHT = 1080;

// Más reflexiones
maxDepth = 5;

// Compilar con optimizaciones
make USERCPPFLAGS="-O3 -march=native -fopenmp"
```

## Visualizar Resultados

### En Linux

```bash
# Ver BMP
eog escena_principal.bmp
# o
xdg-open escena_principal.bmp

# Ver PPM
gimp escena_principal.ppm
```

### Convertir formatos

```bash
# BMP a PNG (requiere ImageMagick)
convert escena_principal.bmp escena_principal.png

# PPM a JPG
convert escena_principal.ppm -quality 95 escena_principal.jpg
```

## Troubleshooting

### Imagen muy oscura
- Aumentar intensidad de luces: `glm::vec3(2.0f)`
- Aumentar luz ambiental: `scene.setAmbientLight(glm::vec3(0.2f))`
- Reducir atenuación de luces

### Imagen muy brillante (sobreexpuesta)
- Reducir intensidad de luces
- Aumentar atenuación
- Reducir coeficientes especulares

### Sombras muy duras
- Es normal con luces puntuales
- Para sombras suaves se requieren area lights (no implementadas)

### Render muy lento
- Reducir resolución
- Reducir profundidad de reflexiones
- Reducir número de objetos o luces
- Asegurarse de compilar con `-O3 -fopenmp`

### Compilación falla
```bash
# Instalar dependencias
sudo apt-get install g++ libglm-dev libomp-dev

# Verificar versión de g++
g++ --version  # Debe soportar C++17
```
