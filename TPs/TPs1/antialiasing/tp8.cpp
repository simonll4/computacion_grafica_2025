// CGyAV - Práctico 8: Antialiasing por Supersampling 3× con Bresenham
// Compilar: g++ -std=c++17 -O2 tp8.cpp -o tp8
//
// Uso:
//   ./tp8 salida.ppm  W  H   x0 y0  x1 y1   [offX offY]   [R G B]   [R2 G2 B2]
//
//   - W H        : tamaño del lienzo en píxeles (coordenadas de usuario)
//   - (x0,y0)-(x1,y1): extremos de la línea a suavizar (AA)
//   - [offX offY]: (opcional) offset para dibujar una SEGUNDA línea SIN AA
//                  con extremos (x0+offX,y0+offY)-(x1+offX,y1+offY)
//   - [R G B]    : color de la línea AA (default: 0 0 0 negro)
//   - [R2 G2 B2] : color de la línea sin AA (default: 255 0 0 rojo)
//
// Notas:
// - Origen de USUARIO: abajo-izquierda (x→derecha, y→ARRIBA).
// - Se usa image_ppm.h con Image, RGB, putUser(x,y,RGB), writePPM(path).
// - Antialiasing por supersampling 3×: rasterizamos la línea en una trama triplicada
//   y luego promediamos cada bloque 3×3 → cobertura/9 → mezcla con el fondo.
//
// Idea matemática: color_final = (1 - cobertura)*FONDO + cobertura*COLOR_LINEA
//   con cobertura ∈ {0..9}/9 proveniente de la cuenta de subpíxeles activos.

#include <bits/stdc++.h>
#include "../common/image_ppm.h"

static inline RGB rgbClamp(int r, int g, int b) {
    auto sat = [](int v){ return (v < 0 ? 0 : (v > 255 ? 255 : v)); };
    return RGB(uint8_t(sat(r)), uint8_t(sat(g)), uint8_t(sat(b)));
}

// ------------------- Bresenham (entero) -------------------
// Dibuja en trama "base": sobre Image con putUser (SIN AA)
static void drawLineBresenhamBase(Image& img, int x0, int y0, int x1, int y1, const RGB& col) {
    int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    for (;;) {
        img.putUser(x0, y0, col);
        if (x0 == x1 && y0 == y1) break;
        int e2 = err << 1;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

// Dibuja en trama "super" (W*3 x H*3) activando subpíxeles (AA)
static void drawLineBresenhamSuper(std::vector<uint8_t>& super, int W3, int H3,
                                   int x0, int y0, int x1, int y1)
{
    auto plot = [&](int x, int y){
        if (x >= 0 && x < W3 && y >= 0 && y < H3) super[y*W3 + x] = 1;
    };

    int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    for (;;) {
        plot(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = err << 1;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

// ------------------- Downsampling 3×3 con mezcla -------------------
static inline uint8_t lerpU8(uint8_t a, uint8_t b, double t) {
    double v = (1.0 - t)*double(a) + t*double(b);
    int vi = int(std::lround(v));
    if (vi < 0) vi = 0; else if (vi > 255) vi = 255;
    return uint8_t(vi);
}

int main(int argc, char** argv) {
    if (argc < 8) {
        std::cerr
            << "Uso: ./tp8 salida.ppm W H x0 y0 x1 y1 [offX offY] [R G B] [R2 G2 B2]\n"
            << "Ej.: ./tp8 aa.ppm 400 300   30 30  360 250   8 8   0 0 0   255 0 0\n";
        return 1;
    }

    const std::string outPath = argv[1];
    const int W  = std::atoi(argv[2]);
    const int H  = std::atoi(argv[3]);
    int x0 = std::atoi(argv[4]), y0 = std::atoi(argv[5]);
    int x1 = std::atoi(argv[6]), y1 = std::atoi(argv[7]);

    if (W <= 0 || H <= 0) { std::cerr << "Error: W y H deben ser > 0.\n"; return 1; }

    // Defaults
    int offX = 0, offY = 0;
    RGB colAA = rgbClamp(0,0,0);       // línea AA
    RGB colNoAA = rgbClamp(255,0,0);   // línea sin AA
    const RGB bg = rgbClamp(255,255,255);

    int idx = 8;
    if (argc >= idx+2) { offX = std::atoi(argv[idx]); offY = std::atoi(argv[idx+1]); idx += 2; }
    if (argc >= idx+3) { colAA = rgbClamp(std::atoi(argv[idx]), std::atoi(argv[idx+1]), std::atoi(argv[idx+2])); idx += 3; }
    if (argc >= idx+3) { colNoAA = rgbClamp(std::atoi(argv[idx]), std::atoi(argv[idx+1]), std::atoi(argv[idx+2])); idx += 3; }

    // 1) Trama supermuestreada 3×
    const int W3 = W * 3;
    const int H3 = H * 3;
    std::vector<uint8_t> super(W3 * H3, 0);

    // Dibujar la línea AA en la trama 3× (escalamos por 3 las coordenadas)
    drawLineBresenhamSuper(super, W3, H3, x0*3, y0*3, x1*3, y1*3);

    // 2) Downsampling 3×3: mezclamos con el fondo y escribimos en la imagen final
    Image img(W, H, bg);

    for (int y = 0; y < H; ++y) {
        int sy = y * 3;
        for (int x = 0; x < W; ++x) {
            int sx = x * 3;

            int count = 0;
            // sumar cobertura del bloque 3×3
            for (int j = 0; j < 3; ++j) {
                int yy = sy + j;
                if (yy < 0 || yy >= H3) continue;
                const uint8_t* row = &super[yy * W3];
                for (int i = 0; i < 3; ++i) {
                    int xx = sx + i;
                    if (xx < 0 || xx >= W3) continue;
                    count += row[xx];
                }
            }
            double cov = double(count) / 9.0; // 0..1

            // mezcla por componente
            RGB out;
            out.r = lerpU8(bg.r, colAA.r, cov);
            out.g = lerpU8(bg.g, colAA.g, cov);
            out.b = lerpU8(bg.b, colAA.b, cov);

            img.putUser(x, y, out);
        }
    }

    // 3) (Opcional) Línea SIN AA con un offset, para comparar
    if (offX != 0 || offY != 0) {
        drawLineBresenhamBase(img, x0 + offX, y0 + offY, x1 + offX, y1 + offY, colNoAA);
    }

    // 4) Guardar
    img.writePPM(outPath);

    std::cout << "---- Supersampling 3x (Práctico 8) ----\n"
              << "Lienzo: " << W << "x" << H << "\n"
              << "Linea AA: (" << x0 << "," << y0 << ") -> (" << x1 << "," << y1 << ")  color=("
              << int(colAA.r) << "," << int(colAA.g) << "," << int(colAA.b) << ")\n"
              << "Linea no-AA offset=(" << offX << "," << offY << ") color=("
              << int(colNoAA.r) << "," << int(colNoAA.g) << "," << int(colNoAA.b) << ")\n"
              << "Salida: " << outPath << "\n";
    return 0;
}
