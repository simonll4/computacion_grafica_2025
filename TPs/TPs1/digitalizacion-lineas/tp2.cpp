// tp2.cpp - CGyAV - Práctico 2
// Compilar: g++ -std=c++17 -O2 tp2.cpp -o tp2
// Uso:      ./tp2 salida.ppm W H x0 y0 x1 y1 x2 y2 x3 y3
//
// Funcionalidad:
//  - Origen de usuario: (0,0) abajo-izquierda (y crece hacia ARRIBA).
//  - PPM P3: filas se escriben de superior -> inferior (raster estándar).
//  - Dibuja la MISMA recta con DDA (rojo) y Bresenham (verde).
//  - Dibuja una SEGUNDA recta CON MISMA PENDIENTE con DDA (magenta) y Bresenham (cyan).
//  - Coincidencias DDA vs Bresenham sobre una misma recta se marcan en AMARILLO.
//  - Estadísticas impresas por stderr.
//  - Fondo blanco en todas las imágenes generadas.

#include "../common/image_ppm.h"
#include <iostream>
#include <algorithm>
#include <climits>
#include <cmath>

static inline int clampi(int v, int lo, int hi)
{
    return (v < lo ? lo : (v > hi ? hi : v));
}

// ========================= DDA (Digital Differential Analyzer) =========================
// Dibuja TODOS los casos: |m|<1, |m|>1, m<0, horizontales, verticales.
// Cuenta píxeles pintados (devuelve cantidad).
size_t drawLineDDA(Image &img, int x0, int y0, int x1, int y1, RGB color)
{
    long dx = x1 - x0;
    long dy = y1 - y0;
    long steps = std::max(std::labs(dx), std::labs(dy));
    if (steps == 0)
    {
        img.putUser(x0, y0, color);
        return 1;
    }
    double xInc = double(dx) / double(steps);
    double yInc = double(dy) / double(steps);
    double x = double(x0);
    double y = double(y0);

    size_t count = 0;
    for (long i = 0; i <= steps; ++i)
    {
        int xi = int(std::floor(x + 0.5));
        int yi = int(std::floor(y + 0.5));
        img.putUser(xi, yi, color);
        ++count;
        x += xInc;
        y += yInc;
    }
    return count;
}

// ========================= Bresenham  =========================
// Implementación clásica extendida a los 8 octantes, y casos H/V.
// Cuenta píxeles pintados (devuelve cantidad).
size_t drawLineBresenham(Image &img, int x0, int y0, int x1, int y1, RGB color)
{
    // Casos degenerados (punto)
    if (x0 == x1 && y0 == y1)
    {
        img.putUser(x0, y0, color);
        return 1;
    }

    size_t count = 0;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    // Caso vertical pura
    if (dx == 0)
    {
        for (int y = y0; y != y1 + sy; y += sy)
        {
            img.putUser(x0, y, color);
            ++count;
        }
        return count;
    }
    // Caso horizontal pura
    if (dy == 0)
    {
        for (int x = x0; x != x1 + sx; x += sx)
        {
            img.putUser(x, y0, color);
            ++count;
        }
        return count;
    }

    // Octantes generales
    bool steep = (dy > dx);
    if (steep)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        std::swap(dx, dy);
        std::swap(sx, sy);
    }

    int err = dx / 2;
    int y = y0;

    for (int x = x0; x != x1 + sx; x += sx)
    {
        if (steep)
            img.putUser(y, x, color);
        else
            img.putUser(x, y, color);
        ++count;

        err -= dy;
        if (err < 0)
        {
            y += sy;
            err += dx;
        }
    }
    return count;
}

// ============ Utilidades para comparación de pendientes ============
// Chequea misma pendiente SIN usar floats: dy1/dx1 == dy2/dx2  <=>  dy1*dx2 == dy2*dx1
bool sameSlope(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
    long long dx1 = (long long)x1 - x0;
    long long dy1 = (long long)y1 - y0;
    long long dx2 = (long long)x3 - x2;
    long long dy2 = (long long)y3 - y2;
    // Considerar verticales/horizontales:
    if (dx1 == 0 && dx2 == 0)
        return true; // ambas verticales
    if (dy1 == 0 && dy2 == 0)
        return true; // ambas horizontales
    if (dx1 == 0 || dx2 == 0)
        return false; // sólo una vertical
    // Mismo signo y misma razón
    return (dy1 * dx2) == (dy2 * dx1);
}

// Función para dibujar Bresenham con mezcla
std::pair<size_t, size_t> drawBresenhamWithMix(Image &img, int x0, int y0, int x1, int y1,
                                               RGB incoming, RGB mixColor)
{
    size_t painted = 0, mixed = 0;

    if (x0 == x1 && y0 == y1)
    {
        // single pixel
        int ys = img.H - 1 - y0;
        RGB before = img.px[ys * img.W + x0];
        img.putUserMix(x0, y0, incoming, mixColor);
        RGB after = img.px[ys * img.W + x0];
        if (!(after == incoming) && !(after == before))
            ++mixed;
        ++painted;
        return {painted, mixed};
    }

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    if (dx == 0)
    {
        for (int y = y0; y != y1 + sy; y += sy)
        {
            int ys = img.H - 1 - y;
            RGB before = img.px[ys * img.W + x0];
            img.putUserMix(x0, y, incoming, mixColor);
            RGB after = img.px[ys * img.W + x0];
            if (!(after == incoming) && !(after == before))
                ++mixed;
            ++painted;
        }
        return {painted, mixed};
    }
    if (dy == 0)
    {
        for (int x = x0; x != x1 + sx; x += sx)
        {
            int ys = img.H - 1 - y0;
            RGB before = img.px[ys * img.W + x];
            img.putUserMix(x, y0, incoming, mixColor);
            RGB after = img.px[ys * img.W + x];
            if (!(after == incoming) && !(after == before))
                ++mixed;
            ++painted;
        }
        return {painted, mixed};
    }

    bool steep = (dy > dx);
    if (steep)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        std::swap(dx, dy);
        std::swap(sx, sy);
    }

    int err = dx / 2;
    int y = y0;
    for (int x = x0; x != x1 + sx; x += sx)
    {
        int ux = steep ? y : x;
        int uy = steep ? x : y;
        if (img.inUserBounds(ux, uy))
        {
            int ys = img.H - 1 - uy;
            RGB before = img.px[ys * img.W + ux];
            img.putUserMix(ux, uy, incoming, mixColor);
            RGB after = img.px[ys * img.W + ux];
            if (!(after == incoming) && !(after == before))
                ++mixed;
        }
        ++painted;
        err -= dy;
        if (err < 0)
        {
            y += sy;
            err += dx;
        }
    }
    return {painted, mixed};
}

int main(int argc, char **argv)
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    if (argc != 12)
    {
        std::cerr << "Uso:\n  " << argv[0] << " salida.ppm W H x0 y0 x1 y1 x2 y2 x3 y3\n";
        std::cerr << "Ejemplo:\n  " << argv[0] << " out.ppm 400 300  10 10  350 200   10 50  350 240\n";
        std::cerr << "  (La segunda recta DEBE tener la MISMA pendiente que la primera).\n";
        return 1;
    }

    std::string outPath = argv[1];
    int W = std::max(1, std::atoi(argv[2]));
    int H = std::max(1, std::atoi(argv[3]));

    auto to_i = [](const char *s) -> int
    {
        long long v = std::atoll(s);
        return (int)clampi((int)v, INT_MIN, INT_MAX);
    };

    int x0 = to_i(argv[4]);
    int y0 = to_i(argv[5]);
    int x1 = to_i(argv[6]);
    int y1 = to_i(argv[7]);

    int x2 = to_i(argv[8]);
    int y2 = to_i(argv[9]);
    int x3 = to_i(argv[10]);
    int y3 = to_i(argv[11]);

    if (W <= 0 || H <= 0)
    {
        std::cerr << "Error: dimensiones invalidas.\n";
        return 1;
    }

    // Colores - fondo blanco
    const RGB BG{255, 255, 255}; // fondo: blanco
    const RGB DDA1{255, 0, 0};   // recta 1 con DDA: rojo
    const RGB BRE1{0, 255, 0};   // recta 1 con Bresenham: verde
    const RGB MIX1{255, 255, 0}; // coincidencias en recta 1: amarillo
    const RGB DDA2{255, 0, 255}; // recta 2 con DDA: magenta
    const RGB BRE2{0, 255, 255}; // recta 2 con Bresenham: cyan
    const RGB MIX2{0, 0, 0};     // coincidencias en recta 2: negro (resalta sobre magenta/cyan)

    Image img(W, H, BG);

    // --------- Recta 1: misma línea con ambos algoritmos ----------
    // Para poder marcar coincidencias visuales, pintamos primero DDA y luego combinamos Bresenham con MIX.
    // (Así, si Bresenham cae en un pixel ya rojo, lo convertimos a amarillo).
    size_t cntDDA1 = drawLineDDA(img, x0, y0, x1, y1, DDA1);

    // Usar la función común para dibujar Bresenham con mezcla
    auto [cntBRE1, mix1] = drawBresenhamWithMix(img, x0, y0, x1, y1, BRE1, MIX1);

    // --------- Recta 2: misma pendiente, distinto origen ----------
    if (!sameSlope(x0, y0, x1, y1, x2, y2, x3, y3))
    {
        std::cerr << "AVISO: La segunda recta NO tiene la misma pendiente que la primera.\n";
    }

    size_t cntDDA2 = drawLineDDA(img, x2, y2, x3, y3, DDA2);
    auto [cntBRE2, mix2] = drawBresenhamWithMix(img, x2, y2, x3, y3, BRE2, MIX2);

    // Guardar
    img.writePPM(outPath);

    // Resumen por stderr
    std::cerr << "---- Comparacion DDA vs Bresenham ----\n";
    std::cerr << "Recta 1: DDA=" << cntDDA1 << " px,  Bresenham=" << cntBRE1
              << " px,  Coincidencias(visuales)=" << mix1 << "\n";
    std::cerr << "Recta 2: DDA=" << cntDDA2 << " px,  Bresenham=" << cntBRE2
              << " px,  Coincidencias(visuales)=" << mix2 << "\n";
    std::cerr << "Colores: DDA1=rojo, BRE1=verde, mix1=amarillo;  DDA2=magenta, BRE2=cyan, mix2=negro.\n";
    std::cerr << "Origen usuario: abajo-izquierda. PPM: filas superior->inferior.\n";
    std::cerr << "Fondo: blanco.\n";
    return 0;
}