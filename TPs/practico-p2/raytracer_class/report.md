# Trazado de Rayos con Fundamentos de CGyAV

## Enfoque basado en las clases

Para rehacer el trabajo práctico se revisaron los apuntes y diapositivas de la materia **Computación Gráfica y Ambientes Virtuales (CGyAV)**.  Allí se abordaron conceptos fundamentales como los sistemas de coordenadas de modelo, vista y proyección, la interpolación de vectores normales, el modelo de iluminación de Phong y la diferencia entre rasterización y trazado de rayos.  Partiendo de ese material, se reorganizó el trazador de rayos entregado anteriormente para alinearlo con la metodología vista en clase:

* Se implementó una **cámara pinhole** siguiendo la derivación del sistema de coordenadas de vista: a partir de un punto de vista (`lookFrom`), un punto objetivo (`lookAt`) y un vector `up` se construyen los ejes ortonormales `u`, `v` y `w` del espacio de cámara.  El tamaño del plano de proyección se calcula a partir del **campo de visión vertical** y el **ratio de aspecto**, y los rayos primarios se generan usando ecuaciones paramétricas.  Esta separación en una clase `Camera` mantiene coherencia con los temas de transformaciones de vista tratados en clase.
* Se mantuvo el modelo de iluminación **Phong** con componentes ambiente, difusa y especular【249099786867922†L210-L236】, pero se reorganizó el código para que cada material defina sus coeficientes y exponentes de brillo.  Las fórmulas de reflexión y refracción se implementaron manualmente, respetando los conceptos de ley de Snell y reflexión interna total【960968147214186†L160-L177】【427411398350678†L95-L105】.
* La estructura modular del proyecto se mantuvo y se mejoró con la incorporación de una **clase de cámara** y de un **Makefile** para automatizar la compilación.  El uso de `std::shared_ptr` y clases abstractas para los objetos permite extender la escena con nuevos tipos de primitivas, en línea con el diseño orientado a objetos explicado en los apuntes.

## Organización del proyecto

El directorio `raytracer_class/` contiene todo el código fuente y archivos de apoyo.  La estructura es la siguiente:

| Carpeta/Archivo | Contenido |
|-----------------|-----------|
| `include/rt/core/vec3.hpp` | Envoltura de vectores 3D basada en GLM (dot, cross, normalize, reflect, refract, etc.). |
| `include/rt/core/ray.hpp` | Rayo paramétrico con origen y dirección. |
| `include/rt/core/camera.hpp` | Clase de cámara que construye ejes de vista y genera rayos primarios. |
| `include/rt/scene/material.hpp` | Tipo de material (difuso, metálico, dieléctrico) y parámetros (albedo, ks/kd/ka, fuzz, índice de refracción, absorción). |
| `include/rt/scene/object.hpp` | Clase base abstracta de objetos + `HitRecord`. |
| `include/rt/geom/sphere.hpp`, `include/rt/geom/triangle.hpp`, `include/rt/geom/plane.hpp` | Primitivas geométricas con sus algoritmos de intersección. |
| `include/rt/scene/light.hpp` | Luz puntual con posición e intensidad RGB. |
| `include/rt/scene/scene.hpp` | Escena: listas de objetos/luces y firma de `traceRay`. |
| `include/rt/io/image_ppm.hpp` | Escritura de imágenes en formato PPM. |
| `src/renderer.cpp` | Bucle de render por píxel (muestreo por píxel y corrección gamma). |
| `src/scene.cpp` | Implementación de `Scene::traceRay` (Phong, sombras, reflexión, refracción). |
| `src/scenes.cpp` | Construcción de escenas "base" y "libre" y sus cámaras. |
| `src/main.cpp` | Programa principal: renderiza ambas escenas (base y libre) sin argumentos y guarda `output/base.ppm` y `output/libre.ppm`. |
| `Makefile` | Script de compilación que genera el ejecutable `build/raytracer`. |
| `output/` | Carpeta destinada a almacenar las imágenes generadas (`base.ppm`, `libre.ppm`). |

Esta organización facilita la lectura, mantenimiento y escalabilidad del código.  Cada módulo aborda una responsabilidad concreta y puede modificarse o ampliarse sin afectar al resto del sistema.

## Conceptos matemáticos implementados

### Sistema de coordenadas de cámara

En las clases se insistió en la importancia de comprender los distintos espacios de coordenadas.  La clase `Camera` implementa el **espacio de cámara** construyendo un trihedro ortonormal `(u, v, w)`: `w` apunta del observador hacia la escena, `u` es perpendicular a `w` y está alineado con el eje horizontal de la imagen, y `v` es el eje vertical.  A partir de estos ejes y del campo de visión vertical se calcula el tamaño del plano de proyección; los rayos se obtienen interpolando entre el **esquina inferior izquierda** y los vectores horizontales y verticales.  Este procedimiento respeta las fórmulas vistas en clase para las proyecciones de perspectiva.

### Iluminación de Phong y modelo local

El modelo de Phong combina componentes ambiente, difusa y especular para aproximar el comportamiento de la luz sobre superficies.  La componente especular se calcula elevando al exponente de brillo el coseno del ángulo entre la dirección de reflexión y el observador【249099786867922†L210-L236】, mientras que la componente difusa depende del producto escalar entre la normal y la dirección hacia la luz.  En el código se asigna un pequeño aporte ambiente (5 %) y se permite ajustar el brillo para cada material.

### Reflexión y refracción

Para materiales metálicos se aplica la ley de la reflexión: `R = I − 2(N·I)N`【427411398350678†L95-L105】.  En materiales dieléctricos se calcula la refracción mediante la ley de Snell, teniendo en cuenta los índices de refracción internos y externos; cuando el ángulo de incidencia supera el ángulo crítico se produce **reflexión interna total**, en cuyo caso se usa exclusivamente la reflexión【960968147214186†L160-L177】.  Para ponderar la cantidad de luz reflejada y transmitida se utiliza la aproximación de **Schlick**, que es una versión simplificada de las ecuaciones de Fresnel【427411398350678†L506-L520】.

### Recursión, sombras y anti‑aliasing

El trazado de rayos se implementa de manera recursiva: cada vez que un rayo impacta sobre un objeto se calcula la iluminación local y, dependiendo del material, se genera un nuevo rayo reflejado o refractado.  La recursión se limita a una profundidad máxima configurable para evitar ciclos infinitos【427411398350678†L205-L216】.  Para simular sombras se envía un rayo hacia cada luz y se comprueba si hay objetos interpuestos.  El anti‑aliasing se consigue tomando varias muestras aleatorias por píxel y promediando los resultados; además, se aplica corrección gamma (γ = 2.2) a los valores RGB antes de escribir la imagen.

## Mejoras respecto a la versión anterior

1. **Separación de cámara:** al introducir la clase `Camera` se encapsuló la lógica de construcción de rayos, evitando código duplicado y facilitando la modificación del punto de vista y del campo de visión.  Esta mejora sigue el patrón de separar responsabilidades y se inspira en las transformaciones de vista estudiadas en clase.
2. **Makefile**: se añadió un archivo `Makefile` para automatizar la compilación.  Basta ejecutar `make` en la raíz del proyecto para generar el ejecutable; `make clean` elimina los objetos y binarios.  Esto cumple con las buenas prácticas de desarrollo recomendadas en la materia.
3. **Comentarios y documentación**: se ampliaron los comentarios para explicar la relación entre el código y los conceptos teóricos.  Cada módulo incluye un encabezado que describe su propósito, lo cual mejora la legibilidad y sirve como guía para futuras extensiones.

## Instrucciones de compilación y uso

Para compilar el proyecto se necesita un compilador compatible con C++17.  Desde la carpeta `raytracer_class` se ejecuta:

```bash
make
```

Esto generará el ejecutable `build/raytracer`.  Para limpiar los archivos temporales se puede ejecutar `make clean`.

Ejecución (sin argumentos):

```bash
./build/raytracer
```

El programa renderiza ambas escenas y escribe dos archivos PPM en `output/`:

- `output/base.ppm`
- `output/libre.ppm`

La resolución, número de muestras por píxel y profundidad recursiva se pueden ajustar editando los valores por defecto al inicio de `src/main.cpp`.

## Resultados y reflexión

La nueva versión produce la misma escena base especificada en el examen: un plano de suelo, tres esferas (difusa, metálica y dieléctrica) y un triángulo verde, iluminados por una luz puntual.  La incorporación de la clase `Camera` no altera el resultado visual pero proporciona una implementación más alineada con el contenido del curso.  El siguiente croquis muestra la configuración de la escena con la cámara y los objetos:

![Croquis de la escena]({{file:file-XocCcsyMtvEAFCRdvjZkCM}})

La imagen renderizada evidencia sombras, reflejos y refracción correctamente.  La esfera metálica refleja el entorno, la esfera de vidrio refracta y refleja parcialmente según la aproximación de Schlick, y la esfera difusa se ilumina de manera mate.  El triángulo verde sirve como referencia para comprobar la refracción.

### Reflexión personal (primera persona)

> **Integrando la teoría de clase:** al revisar las diapositivas comprendí mejor cómo se construye el espacio de cámara a partir de los vectores `lookFrom`, `lookAt` y `up`.  Implementar la clase `Camera` fue un ejercicio valioso para trasladar esos conceptos teóricos al código.  También reforcé el uso de transformaciones y el papel del campo de visión en la percepción de profundidad.
>
> **Mejoras prácticas:** escribir un `Makefile` me permitió automatizar la compilación y entender mejor cómo se enlazan los distintos módulos.  Documentar cuidadosamente cada clase me obligó a reflexionar sobre la relación entre la teoría y la práctica.  Aunque el motor de ray tracing sigue siendo sencillo, ahora está más estructurado y listo para ampliarse con más tipos de objetos o efectos (por ejemplo, fuentes de luz múltiples o texturas procedurales).
>
> **Desafíos encontrados:** el manejo de la refracción y la reflexión interna total continúa siendo la parte más compleja.  Tuve que repasar la derivación de la ley de Snell y ajustar el cálculo para evitar artefactos.  Además, comprobar la corrección del sistema de coordenadas de cámara requirió visualizar mentalmente los ejes `u`, `v` y `w` y compararlo con los ejemplos de las clases.

En conclusión, esta versión del proyecto no solo cumple con los requisitos del segundo parcial sino que además integra de manera explícita los conceptos estudiados en clase, mejorando la organización y la capacidad de expansión del código.
