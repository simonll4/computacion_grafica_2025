// CGyAV - Práctico 1: Bresenham extendido
//
// Compilar: g++ -std=c++17 -O2 tp1.cpp -o tp1
// Uso:      ./tp1 x0 y0 x1 y1 salida.ppm [W H]
// Ejemplo:  ./tp1 10 80 200 400 recta.ppm 500 500
//
// Notas:
// - Origen del usuario: (0,0) abajo-izquierda. y crece hacia ARRIBA.
// - El archivo PPM se escribe con la fila 0 en la parte superior, por lo que se hace flip vertical al volcar cada píxel.
// - Si se pasan W H, se dibuja en ese lienzo (clipping: fuera de [0..W-1]x[0..H-1] no pinta).
// - Si NO se pasan W H, se autocalcula un lienzo que encuadre la recta con margen.
// - Por consigna, NO se dibujan líneas horizontales (y0==y1) ni verticales (x0==x1).
// - Fondo blanco en todas las imágenes generadas.

#include "../common/image_ppm.h"
#include <algorithm>
#include <cstdint>
#include <climits>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>

// =================== Bresenham Para Todos los Casos ===================
// Implementación didáctica basada en la regla del punto medio.
// Cubre todos los octantes separando los casos |m| <= 1 y |m| > 1.
bool drawLineBresenham(Image &img, int x0, int y0, int x1, int y1, RGB color)
{
    // 1) Criterio del TP1: excluir horizontales y verticales
    if (y0 == y1)
        return false; // horizontal
    if (x0 == x1)
        return false; // vertical

    // 2) Diferencias absolutas y signos (cubren 4 cuadrantes / 8 octantes)
    const int dx = std::abs(x1 - x0);
    const int dy = std::abs(y1 - y0);
    const int sx = (x0 < x1) ? 1 : -1; // sentido en x
    const int sy = (y0 < y1) ? 1 : -1; // sentido en y

    // 3) Punto de partida
    int x = x0;
    int y = y0;

    // 4) Elegir eje principal según la pendiente
    if (dx >= dy)
    {
        // ----- CASO A: |m| <= 1 → avanzar en x -----
        // Variable de decisión (D0) y sus incrementos:
        int des_var = 2 * dy - dx;         // D0 = 2Δy - Δx
        const int dd_ady = 2 * dy;         // ΔD si elijo pixel adyacente (E/W)
        const int dd_asup = 2 * (dy - dx); // ΔD si elijo pixel diagonal (NE/SE o NW/SW)
        const int steps = dx;              // cantidad de iteraciones sobre el eje principal

        for (int i = 0; i <= steps; ++i)
        {
            // Pintar en coordenadas de usuario (y hacia arriba).
            // putUser hace el flip vertical al espacio de imagen PPM.
            img.putUser(x, y, color);

            // Regla de decisión incremental:
            // Si D < 0 → el punto medio cae por debajo → elegir adyacente (solo x)
            // Si D >= 0 → el punto medio cae por arriba → elegir diagonal (ajusto y)
            if (des_var < 0)
            {
                des_var += dd_ady; // mantengo y
            }
            else
            {
                des_var += dd_asup;
                y += sy; // ajusto y en el sentido correcto
            }

            x += sx; // avanzar un paso en x siempre
        }
    }
    else
    {
        // ----- CASO B: |m| > 1 → avanzar en y -----
        // Mismos conceptos, con roles x↔y intercambiados:
        int des_var = 2 * dx - dy;         // D0' = 2Δx - Δy
        const int dd_ady = 2 * dx;         // ΔD' si elijo adyacente vertical (N/S)
        const int dd_asup = 2 * (dx - dy); // ΔD' si elijo "lateral" (ajusto x)
        const int steps = dy;

        for (int i = 0; i <= steps; ++i)
        {
            img.putUser(x, y, color);

            if (des_var < 0)
            {
                des_var += dd_ady; // mantengo x
            }
            else
            {
                des_var += dd_asup;
                x += sx; // ajusto x en el sentido correcto
            }

            y += sy; // avanzar un paso en y siempre
        }
    }

    return true;
}

// ================================ main ================================
int main(int argc, char **argv)
{
    // parseo CLI para obtener las coordenadas y el archivo de salida
    if (argc != 6 && argc != 8)
    {
        std::cerr << "Uso:\n  " << argv[0] << " x0 y0 x1 y1 salida.ppm [W H]\n"
                  << "Ejemplos:\n"
                  << "  " << argv[0] << " 0 0 100 100 recta.ppm 500 500\n"
                  << "  " << argv[0] << " 10 80 200 40 recta.ppm\n";
        return 1;
    }

    // Convertir strings a enteros largos para validar rango con seguridad
    auto to_ll = [](const char *s) -> long long
    { return std::atoll(s); };
    long long X0 = to_ll(argv[1]), Y0 = to_ll(argv[2]);
    long long X1 = to_ll(argv[3]), Y1 = to_ll(argv[4]);
    std::string outPath = argv[5];

    // Validar que las coordenadas estén dentro del rango de int
    auto in_int = [](long long v)
    { return v >= INT32_MIN && v <= INT32_MAX; };
    if (!in_int(X0) || !in_int(Y0) || !in_int(X1) || !in_int(Y1))
    {
        std::cerr << "Error: coordenadas fuera de rango int.\n";
        return 1;
    }
    int x0 = int(X0), y0 = int(Y0), x1 = int(X1), y1 = int(Y1);

    // Colores
    RGB bg{255, 255, 255}; // blanco
    RGB fg{0, 0, 0};       // negro

    // (A) Si se proporcionan W y H, se dibuja en ese lienzo
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
    // (B) Si no se proporcionan W y H, se autocalcula un lienzo que encuadre la recta con margen
    else
    {
        // Autodimensionado con margen en coords de usuario (origen abajo-izquierda)
        int xmin = std::min(x0, x1), xmax = std::max(x0, x1);
        int ymin = std::min(y0, y1), ymax = std::max(y0, y1);
        const int MARGIN = 5;

        int W = (xmax - xmin + 1) + 2 * MARGIN;
        int H = (ymax - ymin + 1) + 2 * MARGIN;
        W = std::max(W, 1);
        H = std::max(H, 1);

        Image img(W, H, bg);

        // Trasladar la línea para que el bbox arranque en (MARGIN, MARGIN) en coords usuario
        const int ax0 = x0 - (xmin - MARGIN);
        const int ay0 = y0 - (ymin - MARGIN);
        const int ax1 = x1 - (xmin - MARGIN);
        const int ay1 = y1 - (ymin - MARGIN);

        bool ok = drawLineBresenham(img, ax0, ay0, ax1, ay1, fg); // <- unificada la llamada
        if (!ok)
            std::cerr << "Aviso: linea horizontal o vertical. Por consigna, no se dibuja.\n";

        img.writePPM(outPath);
        return 0;
    }
}