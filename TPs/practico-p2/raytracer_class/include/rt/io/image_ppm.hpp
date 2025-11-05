// -----------------------------------------------------------------------------
//  Imagen en memoria y escritura formato PPM
// -----------------------------------------------------------------------------
//  Define estructura RGB (8 bits por canal) e Image (buffer 2D).
//  El formato PPM (Portable PixMap) es ASCII simple, ideal para debug:
//    P3              <- identificador formato PPM ASCII
//    width height    <- dimensiones
//    255             <- valor máximo por canal
//    R G B ...       <- datos RGB píxel por píxel
//
//  La imagen se almacena en row-major: píxel (x,y) → buffer[y*W + x]
// -----------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>

// Píxel RGB de 8 bits por canal (0-255)
struct RGB
{
    uint8_t r, g, b;
    
    RGB() : r(0), g(0), b(0) {}
    RGB(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
    
    bool operator==(const RGB &o) const { return r == o.r && g == o.g && b == o.b; }
    bool operator!=(const RGB &o) const { return !(*this == o); }
};

// Imagen 2D con buffer contiguo en row-major
struct Image
{
    int W, H;             // Dimensiones (ancho x alto)
    std::vector<RGB> px;  // Buffer de píxeles: px[y*W + x]
    
    // Constructor: crea imagen de tamaño w×h inicializada con color bg
    explicit Image(int w = 1, int h = 1, RGB bg = {255, 255, 255})
        : W(std::max(1, w)), H(std::max(1, h)), px(W * H, bg) {}
    
    // Verifica si (x,y) está dentro de los límites
    inline bool inBounds(int x, int y) const {
        return (0 <= x && x < W && 0 <= y && y < H);
    }
    
    // Escribe píxel en (x,y) si está dentro de límites
    inline void put(int x, int y, RGB c) {
        if (inBounds(x, y)) px[y * W + x] = c;
    }
    
    // Escribe imagen en formato PPM (P3, ASCII)
    void writePPM(const std::string &path) const
    {
        std::ofstream f(path);
        if (!f)
        {
            std::cerr << "Error: no se pudo abrir " << path << " para escribir.\n";
            std::exit(1);
        }
        
        // Encabezado PPM
        f << "P3\n" << W << " " << H << "\n255\n";
        
        // Datos píxel por píxel
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
