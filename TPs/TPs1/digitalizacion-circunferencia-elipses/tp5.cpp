// CGyAV - Práctico 5: Elipse completa (4 cuadrantes) con Punto Medio
// Compilar: g++ -std=c++17 -O2 tp5.cpp -o tp5
// Uso:
//   ./tp5 salida.ppm  W  H   cx  cy   rx  ry   [R G B] ---> ./tp5 elipse1.ppm 400 300 200 150 120 80
//   - salida.ppm : archivo PPM de salida
//   - W H        : tamaño del lienzo en píxeles
//   - cx cy      : centro de la elipse (coordenadas de usuario, origen abajo-izquierda)
//   - rx ry      : semiejes (rx eje X, ry eje Y) enteros >= 0
//   - [R G B]    : color de trazo (opcional, 0..255; default: 0 0 0 negro)
//
// Notas de coordenadas y PPM:
// - Origen del usuario en (0,0) abajo-izquierda. y crece hacia ARRIBA.
// - Se usa image_ppm.h (Image::putUser) que ya hace el flip vertical al raster PPM.
//
// Algoritmo (midpoint ellipse):
// - Se trabaja con la elipse centrada en el ORIGEN, se recorren puntos del primer cuadrante
//   y se reflejan a 4 cuadrantes por simetría (±x, ±y). Luego se traslada al centro (cx,cy).
// - Región 1 (pendiente en magnitud > 1): avanzar principalmente en x.
// - Región 2 (pendiente en magnitud < 1): decrecer principalmente en y.
// - Se evita punto flotante usando una variable de decisión ENTERA escalada por 4.
//   Esto elimina los 0.25/0.5 de los puntos medios clásicos.

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

// Traza los 4 puntos simétricos (cuadrantes) de la elipse para (x,y) relativo al centro (cx,cy)
static inline void plot4(Image &img, int cx, int cy, int x, int y, RGB col)
{
    img.putUser(cx + x, cy + y, col);
    img.putUser(cx - x, cy + y, col);
    img.putUser(cx + x, cy - y, col);
    img.putUser(cx - x, cy - y, col);
}

// Dibuja una elipse completa en “posición estándar” (ejes alineados a X/Y) con centro (cx,cy)
void drawEllipseMidpoint(Image &img, int cx, int cy, int rx, int ry, RGB col)
{
    if (rx < 0 || ry < 0)
        return;

    // Casos degenerados: puntos o segmentos
    if (rx == 0 && ry == 0)
    {
        img.putUser(cx, cy, col);
        return;
    }
    if (rx == 0)
    {
        for (int y = -ry; y <= ry; ++y)
            img.putUser(cx, cy + y, col);
        return;
    }
    if (ry == 0)
    {
        for (int x = -rx; x <= rx; ++x)
            img.putUser(cx + x, cy, col);
        return;
    }

    // Trabajamos con enteros de 64-bit para evitar overflow en rx^2, ry^2, etc.
    const long long rx2 = 1LL * rx * rx;
    const long long ry2 = 1LL * ry * ry;
    const long long twoRx2 = 2LL * rx2;
    const long long twoRy2 = 2LL * ry2;

    // Variables de recorrido (primer cuadrante)
    int x = 0;
    int y = ry;

    // Derivadas (escaladas sin factor 4 aún): dx = 2*ry^2*x, dy = 2*rx^2*y
    long long dx = twoRy2 * x; // = 0
    long long dy = twoRx2 * y; // = 2*rx^2*ry

    // Para la decisión usamos D escalado por 4 para eliminar fracciones:
    // Región 1 init: D1 = 4*(ry^2 - rx^2*ry + 0.25*rx^2) = 4*ry^2 - 4*rx^2*ry + rx^2
    long long D1 = 4LL * ry2 - 4LL * rx2 * y + rx2;

    // También guardamos versiones "por 4" de dx, dy para actualizaciones compactas
    long long dx4 = 4LL * dx; // = 0
    long long dy4 = 4LL * dy; // = 8*rx^2*ry

    // -------- Región 1: avanzar en x mientras 2*ry^2*x <= 2*rx^2*y  (comparación equivalente con escalas por 4)
    while (dx4 <= dy4)
    {
        plot4(img, cx, cy, x, y, col);

        if (D1 < 0)
        {
            // Elegir E (x++)
            x += 1;
            dx += twoRy2;   // dx = 2*ry^2*x
            dx4 = 4LL * dx; // = 8*ry^2*x
            // ΔD1 = 4*dx + 4*ry^2  (usando dx actualizado)
            D1 += dx4 + 4LL * ry2;
        }
        else
        {
            // Elegir SE (x++, y--)
            x += 1;
            y -= 1;
            dx += twoRy2;   // dx = 2*ry^2*x
            dy -= twoRx2;   // dy = 2*rx^2*y
            dx4 = 4LL * dx; // = 8*ry^2*x
            dy4 = 4LL * dy; // = 8*rx^2*y
            // ΔD1 = 4*dx - 4*dy + 4*ry^2  (dx,dy actualizados)
            D1 += dx4 - dy4 + 4LL * ry2;
        }
    }

    // -------- Región 2: decrecer y mientras y>=0
    // D2 = 4*ry^2*(x+0.5)^2 + 4*rx^2*(y-1)^2 - 4*rx^2*ry^2
    long long D2 =
        4LL * ry2 * x * 1LL * x + 4LL * ry2 * x + ry2 +
        4LL * rx2 * y * 1LL * y - 8LL * rx2 * y + 4LL * rx2 - 4LL * rx2 * ry2;

    while (y >= 0)
    {
        plot4(img, cx, cy, x, y, col);

        if (D2 > 0)
        {
            // Elegir S (y--)
            y -= 1;
            dy -= twoRx2;   // dy = 2*rx^2*y
            dy4 = 4LL * dy; // = 8*rx^2*y
            // ΔD2 = 4*rx^2 - 4*dy   (dy actualizado)
            D2 += 4LL * rx2 - dy4;
        }
        else
        {
            // Elegir SE (x++, y--)
            x += 1;
            y -= 1;
            dx += twoRy2;   // dx = 2*ry^2*x
            dy -= twoRx2;   // dy = 2*rx^2*y
            dx4 = 4LL * dx; // = 8*ry^2*x
            dy4 = 4LL * dy; // = 8*rx^2*y
            // ΔD2 = 4*dx - 4*dy + 4*rx^2  (dx,dy actualizados)
            D2 += dx4 - dy4 + 4LL * rx2;
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
