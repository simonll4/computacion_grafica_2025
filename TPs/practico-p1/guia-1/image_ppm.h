// contiene estructuras de imagen y funciones PPM reutilizables

#ifndef GRAPHICS_COMMON_H
#define GRAPHICS_COMMON_H

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>

struct RGB
{
    uint8_t r, g, b;
    
    RGB() : r(0), g(0), b(0) {}
    RGB(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
    
    bool operator==(const RGB &o) const { 
        return r == o.r && g == o.g && b == o.b; 
    }
    
    bool operator!=(const RGB &o) const { 
        return !(*this == o); 
    }
};

struct Image
{
    int W, H;
    std::vector<RGB> px; // row-major; fila 0 = superior (convención PPM)

    explicit Image(int w = 1, int h = 1, RGB bg = {255, 255, 255})
        : W(std::max(1, w)), H(std::max(1, h)), px(W * H, bg) {}

    // Coordenadas de USUARIO: origen abajo-izquierda, y crece hacia arriba
    inline bool inUserBounds(int x, int y) const { 
        return (0 <= x && x < W && 0 <= y && y < H); 
    }

    // Dibuja con origen abajo-izquierda; volcamos con flip vertical (ys = H-1-y)
    inline void putUser(int x, int y, RGB c)
    {
        if (!inUserBounds(x, y))
            return;
        const int ys = H - 1 - y; // flip vertical al volcar en raster
        px[ys * W + x] = c;
    }

    // Mezcla "visual" para marcar coincidencias: si ya hay color 'a' y llega 'b', ponemos 'mix'
    inline void putUserMix(int x, int y, RGB incoming, RGB mixColor)
    {
        if (!inUserBounds(x, y))
            return;
        const int ys = H - 1 - y;
        RGB &dst = px[ys * W + x];
        if (!(dst == incoming))
        {
            // si ya hay algo distinto al incoming, marcamos mix
            dst = mixColor;
        }
        else
        {
            // ya estaba igual; lo dejamos
            dst = incoming;
        }
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
                f << int(c.r) << ' ' << int(c.g) << ' ' << int(c.b);
                f << (x + 1 == W ? '\n' : ' ');
            }
        }
    }
};

#endif // GRAPHICS_COMMON_H