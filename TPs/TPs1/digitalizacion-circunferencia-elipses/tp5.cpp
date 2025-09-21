// CGyAV - Práctico 5: Digitalización de Elipses con Algoritmo de Punto Medio
//
// COMPILACIÓN:
//   g++ -std=c++17 -O2 tp5.cpp -o tp5
//
// USO:
//   ./tp5 salida.ppm W H cx cy rx ry [R G B]
//
// PARÁMETROS:
//   salida.ppm : archivo PPM de salida
//   W H        : dimensiones del lienzo (ancho x alto en píxeles)
//   cx cy      : coordenadas del centro de la elipse
//   rx ry      : semiejes horizontal y vertical (enteros >= 0)
//   [R G B]    : color del trazo RGB (0-255, opcional, default: negro)
//
// EJEMPLOS DE EJECUCIÓN:
//   ./tp5 elipse.ppm 400 300 200 150 120 80
//   ./tp5 elipse_roja.ppm 500 400 250 200 100 60 255 0 0
//   ./tp5 elipse_azul.ppm 600 600 300 300 150 100 0 0 255
//
// SISTEMA DE COORDENADAS:
//   - Origen: (0,0) en esquina inferior-izquierda
//   - Eje X: hacia la derecha
//   - Eje Y: hacia arriba
//   - La función putUser() maneja automáticamente el flip vertical
//
// ALGORITMO DE PUNTO MEDIO PARA ELIPSES:
//   - Ecuación de la elipse: (x/rx)² + (y/ry)² = 1
//   - Se divide en dos regiones según la pendiente de la tangente
//   - Región 1: |pendiente| < 1 (avanzar principalmente en x)
//   - Región 2: |pendiente| > 1 (avanzar principalmente en y)
//   - Usa simetría de 4 puntos para completar toda la elipse
//   - Variables de decisión enteras para evitar aritmética flotante

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include "../common/image_ppm.h"

static inline RGB rgbClamp(int r, int g, int b)
{
    auto sat = [](int v)
    { return (v < 0 ? 0 : (v > 255 ? 255 : v)); };
    return RGB(uint8_t(sat(r)), uint8_t(sat(g)), uint8_t(sat(b)));
}

// ===== FUNCIÓN DE SIMETRÍA: PINTAR 4 PUNTOS =====
// Dada una coordenada (x,y) calculada en el primer cuadrante,
// pinta los 4 puntos simétricos para completar toda la elipse
static inline void plot4(Image &img, int cx, int cy, int x, int y, RGB col)
{
    img.putUser(cx + x, cy + y, col);          // Cuadrante I:  (+x, +y)
    img.putUser(cx - x, cy + y, col);          // Cuadrante II: (-x, +y)
    img.putUser(cx + x, cy - y, col);          // Cuadrante IV: (+x, -y)
    img.putUser(cx - x, cy - y, col);          // Cuadrante III:(-x, -y)
}

// ===== ALGORITMO DE PUNTO MEDIO PARA ELIPSES =====
// Basado en la implementación de referencia, con comentarios paso a paso
void drawEllipseMidpoint(Image &img, int cx, int cy, int rx, int ry, RGB col)
{
    // Validación básica
    if (rx < 0 || ry < 0) return;

    // Casos especiales
    if (rx == 0 && ry == 0) {
        img.putUser(cx, cy, col);
        return;
    }
    if (rx == 0) {
        for (int y = -ry; y <= ry; ++y)
            img.putUser(cx, cy + y, col);
        return;
    }
    if (ry == 0) {
        for (int x = -rx; x <= rx; ++x)
            img.putUser(cx + x, cy, col);
        return;
    }

    // ===== INICIALIZACIÓN =====
    int x = 0;                          // Comenzamos en x = 0
    int y = ry;                         // y = semieje vertical (punto más alto)

    // Precálculos para evitar multiplicaciones repetidas
    int rx2 = rx * rx;                  // rx²
    int ry2 = ry * ry;                  // ry²
    int twoRx2 = 2 * rx2;              // 2*rx²
    int twoRy2 = 2 * ry2;              // 2*ry²

    // Variables para las derivadas (acumuladores incrementales)
    int dx = 0;                         // Acumulador para avances en x
    int dy = twoRx2 * y;               // Acumulador para avances en y = 2*rx²*ry

    // ===== REGIÓN 1: AVANZAR PRINCIPALMENTE EN X =====
    // Condición: dx < dy (equivalente a pendiente > -1)
    // Variable de decisión inicial (multiplicada por 4 para evitar fracciones)
    int d1 = 4 * ry2 - 4 * rx2 * ry + rx2;

    while (dx < dy) {
        // Pintar los 4 puntos simétricos
        plot4(img, cx, cy, x, y, col);

        if (d1 < 0) {
            // CASO 1: d1 < 0 → Elegir píxel ESTE (solo avanzar en x)
            x++;                        // Incrementar x
            dx += twoRy2;              // Actualizar dx = dx + 2*ry²
            d1 += dx + 4 * ry2;        // Actualizar d1
        } else {
            // CASO 2: d1 >= 0 → Elegir píxel SURESTE (avanzar en x y decrementar y)
            x++;                        // Incrementar x
            y--;                        // Decrementar y
            dx += twoRy2;              // Actualizar dx = dx + 2*ry²
            dy -= twoRx2;              // Actualizar dy = dy - 2*rx²
            d1 += dx - dy + 4 * ry2;   // Actualizar d1
        }
    }

    // ===== REGIÓN 2: AVANZAR PRINCIPALMENTE EN Y =====
    // Condición: y >= 0 (hasta llegar al eje x)
    // Nueva variable de decisión para la región 2
    int d2 = (ry2 * (2 * x + 1) * (2 * x + 1) + rx2 * (2 * y - 2) * (2 * y - 2) - 4 * rx2 * ry2);

    while (y >= 0) {
        // Pintar los 4 puntos simétricos
        plot4(img, cx, cy, x, y, col);

        if (d2 > 0) {
            // CASO 1: d2 > 0 → Elegir píxel SUR (solo decrementar y)
            y--;                        // Decrementar y
            dy -= twoRx2;              // Actualizar dy = dy - 2*rx²
            d2 += rx2 - dy;            // Actualizar d2
        } else {
            // CASO 2: d2 <= 0 → Elegir píxel SURESTE (avanzar en x y decrementar y)
            x++;                        // Incrementar x
            y--;                        // Decrementar y
            dx += twoRy2;              // Actualizar dx = dx + 2*ry²
            dy -= twoRx2;              // Actualizar dy = dy - 2*rx²
            d2 += dx - dy + rx2;       // Actualizar d2
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 8)
    {
        std::cerr
            << "Uso: ./tp5 salida.ppm W H cx cy rx ry [R G B]\n"
            << "Ej.: ./tp5 elipse.ppm 400 300 200 150 120 80  0 0 0\n";
        return 1;
    }

    const std::string outPath = argv[1];
    const int W = std::atoi(argv[2]);
    const int H = std::atoi(argv[3]);
    const int cx = std::atoi(argv[4]);
    const int cy = std::atoi(argv[5]);
    const int rx = std::atoi(argv[6]);
    const int ry = std::atoi(argv[7]);

    if (W <= 0 || H <= 0)
    {
        std::cerr << "Error: W y H deben ser > 0.\n";
        return 1;
    }
    if (rx < 0 || ry < 0)
    {
        std::cerr << "Error: rx y ry deben ser >= 0.\n";
        return 1;
    }

    // Color de trazo opcional (default negro)
    RGB stroke = rgbClamp(0, 0, 0);
    if (argc >= 11)
    {
        stroke = rgbClamp(std::atoi(argv[8]), std::atoi(argv[9]), std::atoi(argv[10]));
    }

    // Fondo fijo: blanco
    const RGB bg = rgbClamp(255, 255, 255);

    Image img(W, H, bg);

    // Dibujo de la elipse completa (4 cuadrantes por simetría)
    drawEllipseMidpoint(img, cx, cy, rx, ry, stroke);

    // Escribir PPM
    img.writePPM(outPath);

    // Resumen en stdout
    std::cout << "---- Elipse (Punto Medio) ----\n"
              << "Centro: (" << cx << "," << cy << "), rx=" << rx << ", ry=" << ry << "\n"
              << "Lienzo: " << W << "x" << H << "\n"
              << "Trazo: (" << int(stroke.r) << "," << int(stroke.g) << "," << int(stroke.b) << ")\n"
              << "Fondo: (" << int(bg.r) << "," << int(bg.g) << "," << int(bg.b) << ")\n"
              << "Salida: " << outPath << "\n";
    return 0;
}
