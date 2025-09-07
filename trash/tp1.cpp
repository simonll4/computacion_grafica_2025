
// CGyAV - Práctico 1: Bresenham extendido
//
// Compilar: g++ -std=c++17 -O2 tp1.cpp -o tp1
// Uso:      ./tp1 x0 y0 x1 y1 salida.ppm [W H]
//
// Notas:
// - Origen del usuario: (0,0) abajo-izquierda. y crece hacia ARRIBA.
// - El archivo PPM se escribe con la fila 0 en la parte superior,por lo que se hace flip vertical al volcar cada píxel.
// - Si se pasan W H, se dibuja en ese lienzo (clipping: fuera de [0..W-1]x[0..H-1] no pinta).
// - Si NO se pasan W H, se autocalcula un lienzo que encuadre la recta con margen.
// - Por consigna, NO se dibujan líneas horizontales (y0==y1) ni verticales (x0==x1).

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <cmath>

struct RGB
{
    uint8_t r, g, b;
};

struct Image
{
    int W, H;
    std::vector<RGB> px; // row-major; fila 0 = superior (convención PPM)

    explicit Image(int w = 1, int h = 1, RGB bg = {255, 255, 255})
        : W(std::max(1, w)), H(std::max(1, h)), px(W * H, bg) {}

    inline bool inBounds(int x, int y) const
    {
        return (0 <= x && x < W && 0 <= y && y < H);
    }

    // dibuja con origen abajo-izquierda; volcamos con flip vertical (ys = H-1-y)
    inline void put(int x, int y, RGB c)
    {
        if (!inBounds(x, y))
            return;
        int ys = H - 1 - y; // flip vertical
        px[ys * W + x] = c;
    }

    void writePPM(const std::string &path) const
    {
        std::ofstream f(path);
        if (!f)
        {
            std::cerr << "Error: no se pudo abrir " << path << " para escribir.\n";
            std::exit(1);
        }
        f << "P3\n"
          << W << " " << H << "\n255\n";
        for (int y = 0; y < H; ++y)
        {
            for (int x = 0; x < W; ++x)
            {
                const RGB &c = px[y * W + x];
                f << int(c.r) << ' ' << int(c.g) << ' ' << int(c.b)
                  << (x + 1 == W ? '\n' : ' ');
            }
        }
    }
};

// Bresenham generalizado. Excluye horizontales/verticales.
bool drawLineBresenham(Image &img, int x0, int y0, int x1, int y1, RGB color)
{
    if (y0 == y1)
        return false; // horizontal (excluida)
    if (x0 == x1)
        return false; // vertical   (excluida)

    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = std::abs(y1 - y0);
    int err = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;

    for (int x = x0; x <= x1; ++x)
    {
        if (steep)
            img.put(y, x, color);
        else
            img.put(x, y, color);
        err -= dy;
        if (err < 0)
        {
            y += ystep;
            err += dx;
        }
    }
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 6 && argc != 8)
    {
        std::cerr << "Uso:\n  " << argv[0] << " x0 y0 x1 y1 salida.ppm [W H]\n"
                  << "Ejemplos:\n"
                  << "  " << argv[0] << " 0 0 100 100 recta.ppm 500 500\n"
                  << "  " << argv[0] << " 10 80 200 40 recta.ppm\n";
        return 1;
    }

    auto to_ll = [](const char *s) -> long long
    { return std::atoll(s); };
    long long X0 = to_ll(argv[1]), Y0 = to_ll(argv[2]);
    long long X1 = to_ll(argv[3]), Y1 = to_ll(argv[4]);
    std::string outPath = argv[5];

    auto in_int = [](long long v)
    { return v >= INT32_MIN && v <= INT32_MAX; };
    if (!in_int(X0) || !in_int(Y0) || !in_int(X1) || !in_int(Y1))
    {
        std::cerr << "Error: coordenadas fuera de rango int.\n";
        return 1;
    }
    int x0 = int(X0), y0 = int(Y0), x1 = int(X1), y1 = int(Y1);

    RGB bg{255, 255, 255};
    RGB fg{0, 0, 0};

    if (argc == 8)
    {
        int W = std::max(1, std::atoi(argv[6]));
        int H = std::max(1, std::atoi(argv[7]));
        Image img(W, H, bg);

        bool ok = drawLineBresenham(img, x0, y0, x1, y1, fg);
        if (!ok)
            std::cerr << "Aviso: linea horizontal o vertical. Por consigna, no se dibuja.\n";

        img.writePPM(outPath);
        return 0;
    }
    else
    {
        // Autodimensionado con margen en coords usuario (origen abajo-izquierda)
        int xmin = std::min(x0, x1), xmax = std::max(x0, x1);
        int ymin = std::min(y0, y1), ymax = std::max(y0, y1);
        const int MARGIN = 5;

        int W = (xmax - xmin + 1) + 2 * MARGIN;
        int H = (ymax - ymin + 1) + 2 * MARGIN;
        W = std::max(W, 1);
        H = std::max(H, 1);

        Image img(W, H, bg);

        // Trasladar la línea para que el bbox arranque en (MARGIN, MARGIN) en coords usuario (abajo-izquierda)
        int ax0 = x0 - (xmin - MARGIN);
        int ay0 = y0 - (ymin - MARGIN);
        int ax1 = x1 - (xmin - MARGIN);
        int ay1 = y1 - (ymin - MARGIN);

        bool ok = drawLineBresenham(img, ax0, ay0, ax1, ay1, fg);
        if (!ok)
            std::cerr << "Aviso: linea horizontal o vertical. Por consigna, no se dibuja.\n";

        img.writePPM(outPath);
        return 0;
    }
}