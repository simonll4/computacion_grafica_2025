# Práctico 1 - Guía 2: Elipse con Orientación Arbitraria

## Descripción

Este proyecto implementa una extensión del algoritmo de punto medio para graficar elipses que permite:

1. **Posición arbitraria**: La elipse puede estar centrada en cualquier punto del plano
2. **Orientación arbitraria**: Los ejes de la elipse pueden estar orientados en cualquier dirección mediante rotación

## Características Técnicas

- **Algoritmo**: Punto medio (midpoint) para elipses
- **Precisión**: Uso de enteros escalados por 4 para evitar aritmética de punto flotante
- **Simetría**: Aprovecha la simetría de 4 cuadrantes para eficiencia
- **Transformaciones**: Rotación respecto a un punto arbitrario
- **Formato**: Genera archivos PPM para visualización

## Archivos

- `tp1_ellipse_rotated.cpp`: Implementación principal
- `Makefile`: Archivo de compilación con ejemplos
- `README.md`: Esta documentación

## Compilación

```bash
make
```

## Uso

```bash
./tp1_ellipse_rotated salida.ppm W H cx cy rx ry angle [R G B]
```

### Parámetros

- `salida.ppm`: Archivo PPM de salida
- `W H`: Tamaño del lienzo en píxeles
- `cx cy`: Centro de la elipse (coordenadas de usuario, origen abajo-izquierda)
- `rx ry`: Semiejes (rx eje X, ry eje Y) enteros >= 0
- `angle`: Ángulo de rotación en grados (positivo = antihorario)
- `[R G B]`: Color de trazo (opcional, 0..255; default: 0 0 0 negro)

### Ejemplos

```bash
# Elipse sin rotación (0 grados)
./tp1_ellipse_rotated elipse_0deg.ppm 400 300 200 150 120 80 0 255 0 0

# Elipse rotada 45 grados
./tp1_ellipse_rotated elipse_45deg.ppm 400 300 200 150 120 80 45 0 255 0

# Elipse rotada 90 grados
./tp1_ellipse_rotated elipse_90deg.ppm 400 300 200 150 120 80 90 0 0 255

# Elipse rotada 135 grados
./tp1_ellipse_rotated elipse_135deg.ppm 400 300 200 150 120 80 135 255 0 255
```

## Ejecutar Ejemplos

```bash
make examples
```

## Algoritmo

1. **Generación base**: Se genera la elipse en posición estándar usando el algoritmo de punto medio
2. **Transformación**: Se aplica rotación a cada punto generado usando:
   - `x' = x*cos(θ) - y*sin(θ)`
   - `y' = x*sin(θ) + y*cos(θ)`
3. **Traslación**: Se traslada al centro final (cx, cy)
4. **Dibujo**: Se dibuja el punto rotado y trasladado

## Casos Especiales

- **rx = 0, ry = 0**: Punto único
- **rx = 0**: Línea vertical rotada
- **ry = 0**: Línea horizontal rotada
- **rx = ry**: Círculo rotado

## Dependencias

- `image_ppm.h`: Biblioteca de manejo de imágenes PPM (ubicada en `../TPs1/common/`)
- Compilador C++17 o superior
- Biblioteca matemática (`-lm`)

## Limpieza

```bash
make clean
```

Esto elimina el ejecutable y todos los archivos PPM generados.
