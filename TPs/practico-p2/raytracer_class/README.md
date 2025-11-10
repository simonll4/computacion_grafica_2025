# Raytracer Recursivo

Este proyecto implementa un trazador de rayos recursivo en C++ que calcula iluminación global básica (reflexión y refracción) y sombreado local con el modelo Phong. La escena por defecto incluye cinco esferas, un plano que actúa como suelo y cuatro luces puntuales.

## Requisitos

- Compilador C++17 compatible
- Biblioteca GLM instalada

## Compilación

```bash
make
```

El ejecutable queda en `build/raytracer`.

## Ejecución

Render por defecto (1920x1080, 10 muestras por píxel):

```bash
./build/raytracer
```

Render con resolución personalizada:

```bash
./build/raytracer <ancho> <alto>
```

## Resultado

El render se guarda en `output/render.ppm`. Puedes abrirlo con visores que soporten PPM o convertirlo a PNG usando herramientas como `magick` (ImageMagick):

```bash
magick output/render.ppm output/render.png
```
