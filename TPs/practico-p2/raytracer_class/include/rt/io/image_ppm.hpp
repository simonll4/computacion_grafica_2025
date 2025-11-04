// -----------------------------------------------------------------------------
//  Archivo: image_ppm.hpp
//  Descripción: Estructuras de imagen y escritura en formato PPM (ASCII, P3).
//  El buffer está en orden row-major y `writePPM` vuelca los valores 0..255.
//  La corrección gamma y el promedio por SPP se realizan antes (en renderer).
// -----------------------------------------------------------------------------
#pragma once

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
    bool operator==(const RGB &o) const { return r == o.r && g == o.g && b == o.b; }
    bool operator!=(const RGB &o) const { return !(*this == o); }
};

struct Image
{
    int W, H;
    std::vector<RGB> px; // row-major
    explicit Image(int w = 1, int h = 1, RGB bg = {255, 255, 255})
        : W(std::max(1, w)), H(std::max(1, h)), px(W * H, bg) {}
    inline bool inBounds(int x, int y) const { return (0 <= x && x < W && 0 <= y && y < H); }
    inline void put(int x, int y, RGB c) { if (inBounds(x, y)) px[y * W + x] = c; }
    void writePPM(const std::string &path) const
    {
        std::ofstream f(path);
        if (!f)
        {
            std::cerr << "Error: no se pudo abrir " << path << " para escribir.\n";
            std::exit(1);
        }
        f << "P3\n" << W << " " << H << "\n255\n";
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

// fin
