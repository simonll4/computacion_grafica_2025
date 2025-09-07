// CGyAV - Práctico 3: Circunferencia completa con Punto Medio (Bresenham)
// Compilar: g++ -std=c++17 -O2 tp3.cpp -o tp3
// Uso:
//   ./tp3 salida.ppm  W  H   cx  cy  r   [R G B]
//   - salida.ppm : nombre del archivo PPM de salida
//   - W H        : tamaño del lienzo en píxeles
//   - cx cy r    : centro (coordenadas de usuario) y radio (entero >= 0)
//   - [R G B]    : color de trazo (opcional, 0..255; default: 0 0 0 negro)
//
// Notas de coordenadas:
// - Origen de usuario: (0,0) abajo-izquierda (x→derecha, y→ARRIBA).
// - image_ppm.h provee Image::putUser(x,y,RGB) que hace el flip vertical.
// - Se trazan las 8 simetrías en cada paso para cubrir 360°.
//
// Algoritmo del Punto Medio (Bresenham círculo):
// - Estado inicial en el 2° octante: x=0, y=r, d=1-r.
// - Mientras x <= y: pintar simetrías y actualizar:
//     si d < 0  -> d += 2x + 3;            x++
//     si d >= 0 -> d += 2(x - y) + 5;      x++, y--

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string>
#include "../common/image_ppm.h"

static inline RGB rgbClamp(int r, int g, int b) {
    auto sat = [](int v){ return (v < 0 ? 0 : (v > 255 ? 255 : v)); };
    return RGB(uint8_t(sat(r)), uint8_t(sat(g)), uint8_t(sat(b)));
}

// 8 simetrías alrededor de (cx,cy) para el punto (x,y)
static inline void plot8(Image &img, int cx, int cy, int x, int y, RGB col) {
    img.putUser(cx + x, cy + y, col);
    img.putUser(cx - x, cy + y, col);
    img.putUser(cx + x, cy - y, col);
    img.putUser(cx - x, cy - y, col);

    img.putUser(cx + y, cy + x, col);
    img.putUser(cx - y, cy + x, col);
    img.putUser(cx + y, cy - x, col);
    img.putUser(cx - y, cy - x, col);
}

void drawCircleMidpoint(Image &img, int cx, int cy, int r, RGB col) {
    if (r < 0) return;
    if (r == 0) { img.putUser(cx, cy, col); return; }

    int x = 0;
    int y = r;
    int d = 1 - r;

    plot8(img, cx, cy, x, y, col);

    while (x <= y) {
        if (d < 0) {
            d += (2 * x + 3);
            x += 1;
        } else {
            d += (2 * (x - y) + 5);
            x += 1;
            y -= 1;
        }
        plot8(img, cx, cy, x, y, col);
    }
}

int main(int argc, char** argv) {
    if (argc < 7) {
        std::cerr
            << "Uso: ./tp3 salida.ppm W H cx cy r [R G B]\n"
            << "Ej.: ./tp3 circ.ppm 400 300 200 150 80  0 0 0\n";
        return 1;
    }

    const std::string outPath = argv[1];
    const int W  = std::atoi(argv[2]);
    const int H  = std::atoi(argv[3]);
    const int cx = std::atoi(argv[4]);
    const int cy = std::atoi(argv[5]);
    const int r  = std::atoi(argv[6]);

    if (W <= 0 || H <= 0) {
        std::cerr << "Error: W y H deben ser > 0.\n";
        return 1;
    }
    if (r < 0) {
        std::cerr << "Error: el radio r debe ser >= 0.\n";
        return 1;
    }

    // Color de trazo opcional (default negro)
    RGB stroke = rgbClamp(0,0,0);
    if (argc >= 10) {
        stroke = rgbClamp(std::atoi(argv[7]), std::atoi(argv[8]), std::atoi(argv[9]));
    }

    // Fondo fijo: blanco
    const RGB bg = rgbClamp(255,255,255);

    // Crear imagen y dibujar circunferencia completa
    Image img(W, H, bg);
    drawCircleMidpoint(img, cx, cy, r, stroke);

    // Escribir PPM
    img.writePPM(outPath);

    // Resumen en stdout
    std::cout << "---- Circunferencia (Punto Medio) ----\n"
              << "Centro: (" << cx << "," << cy << "), r=" << r << "\n"
              << "Lienzo: " << W << "x" << H << "\n"
              << "Trazo: (" << int(stroke.r) << "," << int(stroke.g) << "," << int(stroke.b) << ")\n"
              << "Fondo: (" << int(bg.r) << "," << int(bg.g) << "," << int(bg.b) << ")\n"
              << "Salida: " << outPath << "\n";
    return 0;
}
