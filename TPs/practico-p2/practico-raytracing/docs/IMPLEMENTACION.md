# Documentación Técnica - Ray Tracer
## Detalles de Implementación

### 1. Arquitectura General

El ray tracer implementa el algoritmo clásico de trazado de rayos con las siguientes características:

```
Cámara → Rayos primarios → Intersección → Shading → Píxel
                              ↓
                          Sombras/Reflexiones
```

### 2. Generación de Rayos (Camera.h)

La cámara genera rayos usando el modelo pinhole:

```cpp
// Sistema de coordenadas de la cámara
w = normalize(position - lookAt);  // Forward
u = normalize(cross(up, w));       // Right
v = cross(w, u);                   // Up recalculado

// Plano de imagen
planeHeight = 2 * nearPlane * tan(fov/2);
planeWidth = aspectRatio * planeHeight;
```

Para cada píxel (s,t) en [0,1]×[0,1]:
- Se mapea al plano de imagen
- Se genera un rayo desde la cámara hacia ese punto

### 3. Intersección Rayo-Esfera (Sphere.h)

Resuelve la ecuación cuadrática:
```
(P - C)·(P - C) = r²
```
Donde P = O + t·D (ecuación del rayo)

Resulta en: `at² + bt + c = 0`
- a = D·D
- b = 2(O-C)·D
- c = (O-C)·(O-C) - r²

Se toma la solución más cercana t > 0.

### 4. Intersección Rayo-AABB (AABB.h)

Usa el método de slabs (planos paralelos):

```cpp
for cada eje i:
    t_enter = max(t_enter, (min[i] - origin[i]) / dir[i])
    t_exit = min(t_exit, (max[i] - origin[i]) / dir[i])
    
si t_enter > t_exit: no hay intersección
```

La normal se determina según qué plano fue atravesado primero.

### 5. Modelo de Iluminación de Phong

Implementa la ecuación completa:

```
I = I_ambient + Σ(f_att * I_light * (I_diffuse + I_specular))
```

Donde:
- **Ambiental**: `I_a * k_a`
- **Difusa**: `I_p * k_d * max(0, n·l)`
- **Especular**: `I_p * k_s * max(0, v·r)^n`
- **Atenuación**: `1 / (c₁ + c₂d + c₃d²)`

### 6. Sombras

Para cada punto iluminado:
1. Se lanza un rayo hacia cada luz
2. Si intersecta un objeto antes de llegar a la luz → sombra
3. Se usa un epsilon (0.001) para evitar auto-intersección

```cpp
bool inShadow(point, lightPos) {
    Ray shadowRay(point + ε*toLight, toLight);
    return intersect(shadowRay, ε, distance-ε);
}
```

### 7. Reflexiones Especulares

Implementadas recursivamente con límite de profundidad:

```cpp
if (depth < maxDepth && specular > threshold) {
    reflectDir = reflect(rayDir, normal);
    reflectColor = trace(reflectRay, depth+1);
    color = lerp(color, reflectColor, reflectivity);
}
```

### 8. Optimizaciones

#### Paralelización con OpenMP
```cpp
#pragma omp parallel for
for (int j = 0; j < height; ++j) {
    for (int i = 0; i < width; ++i) {
        // Cada thread procesa píxeles independientes
    }
}
```

#### Early Termination
- Se detiene al encontrar la primera intersección válida
- Los objetos se prueban hasta encontrar el más cercano

#### Epsilon para Estabilidad Numérica
- Evita auto-intersecciones en sombras y reflexiones
- Valor típico: 0.001

### 9. Estructura de Datos

```
Scene
├── Objects[] (polimórficos vía Object*)
│   ├── Sphere
│   └── AABB
├── Lights[]
│   └── Light (puntual con atenuación)
└── ambientLight (global)
```

### 10. Pipeline de Renderizado

```
Para cada píxel (i,j):
    1. Generar rayo primario
    2. Buscar intersección más cercana
    3. Si hay intersección:
        a. Calcular normal
        b. Para cada luz:
            - Verificar sombras
            - Calcular Phong (ambiental + difusa + especular)
        c. Si es reflectivo:
            - Trazar rayo reflejado (recursivo)
    4. Si no hay intersección:
        - Color de fondo (cielo)
    5. Clamp y guardar color
```

### 11. Validaciones Implementadas

| Prueba | Objetivo | Resultado |
|--------|----------|-----------|
| Esfera sola | Validar intersección y normales | ✓ Correcto |
| Cubo solo | Validar AABB y normales por cara | ✓ Correcto |
| Sombras | Oclusiones entre objetos | ✓ Proyección correcta |
| Múltiples luces | Suma de contribuciones | ✓ Mezcla aditiva |
| Atenuación | Caída con distancia | ✓ Física correcta |
| Shininess | Variación de brillos | ✓ 4, 16, 64, 256 |
| Reflexiones | Recursión especular | ✓ Hasta depth=3 |

### 12. Formato de Salida

**PPM (Portable Pixel Map)**
- Formato ASCII simple
- Header: P3 width height maxval
- Datos: R G B por píxel

**BMP (Bitmap)**
- Formato binario estándar
- Compatible con todos los visores
- Almacenamiento bottom-up, BGR

### 13. Complejidad Computacional

- **Por píxel**: O(n·m) donde n=objetos, m=luces
- **Total**: O(w·h·n·m) donde w×h = resolución
- **Con reflexiones**: O(w·h·n·m·d) donde d=profundidad

### 14. Posibles Extensiones

1. **Anti-aliasing**: Supersampling o jittering
2. **Texturas**: Mapeo UV en las superficies
3. **BVH**: Acelerar con jerarquías de volúmenes
4. **Más primitivas**: Planos, triángulos, mallas
5. **Refracción**: Ley de Snell para transparencia
6. **Global Illumination**: Path tracing o photon mapping

### 15. Limitaciones Conocidas

- Solo luces puntuales (no área lights)
- Sombras duras únicamente
- Sin motion blur ni depth of field
- Materiales isotrópicos (no anisotropía)
- Sin subsurface scattering

---

## Cumplimiento del Plan Original

✅ **Todos los objetivos del plan han sido implementados exitosamente:**

1. Ray tracing básico desde la cámara
2. Iluminación local de Phong completa
3. Geometría: esfera y cubo AABB
4. Sombras por oclusión directa
5. Atenuación por distancia
6. Múltiples luces con suma por canal
7. Reflexiones especulares (bonus)
8. Validaciones visuales exhaustivas

El ray tracer está listo para su evaluación.
