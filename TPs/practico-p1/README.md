# Practico Primer Pacial

### guia-1/

Contiene el **TP2: DDA vs. Bresenham** - Comparación de algoritmos de rasterización de líneas.

- **`tp2.cpp`**: Implementación principal que compara los algoritmos DDA y Bresenham
- **`image_ppm.h`**: Biblioteca propia para manejo de imágenes en formato PPM
- **`resultados/`**: Directorio con los resultados esperados del TP2
  - `comparacion.txt`: Tabla comparativa punto por punto entre ambos algoritmos
  - `out.ppm`: Imagen generada mostrando las líneas rasterizadas

**Compilación y uso:**

```bash
cd guia-1/
g++ -std=c++17 -O2 tp2.cpp -o tp2
./tp2 x0 y0 x1 y1 salida.ppm [W H] [ox oy]
#EJEMPLO: ./tp2 10 80 200 400 out.ppm 800 600 40 -1
```

**Parámetros:**

- `x0 y0 x1 y1`: coordenadas de inicio y fin de la línea a rasterizar
- `salida.ppm`: nombre del archivo de imagen de salida
- `W H`: tamaño del lienzo. Si se omite, se auto-calcula con margen.
- `ox oy`: traslación aplicada a la 1ª recta para generar la 2ª (misma pendiente, distinto punto).

### guia-2/

Contiene el **TP4: Manejo de Eventos** - Rotación interactiva con pivote usando OpenGL.

- **`tp4/`**: Proyecto completo de OpenGL con rotación interactiva
  - `src/main.cpp`: Programa principal con manejo de eventos de mouse y teclado
  - `src/Shader.cpp`: Clase para manejo de shaders
  - `src/SimpleMesh.cpp`: Clase para manejo de geometría simple
  - `shaders/`: Shaders de vértice y fragmento
  - `include/`: Headers necesarios (GLAD, GLFW, GLM)

**Compilación y uso:**

```bash
cd guia-2/tp4/
make
./build/OGL-program
```

**Controles del TP4:**

- Click izquierdo: Establecer nuevo pivote de rotación
- ESPACIO: Pausar/reanudar rotación
- R: Reset (volver al pivote inicial)
- ESC: Salir
