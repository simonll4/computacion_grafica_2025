// CGyAV - Práctico 2: DDA vs. Bresenham (tabla de puntos + imagen PPM)
//
// Compilar: g++ -std=c++17 -O2 tp2.cpp -o tp2
// Uso:      ./tp2 x0 y0 x1 y1 salida.ppm [W H] [ox oy]
//           - W H   : tamaño del lienzo. Si se omite, se auto-calcula con margen.
//           - ox oy : traslación aplicada a la 1ª recta para generar la 2ª
//                     (misma pendiente, distinto punto).
// Ejemplo:  ./tp2 10 80 200 400 out.ppm 800 600 40 -1
//
// Descripción breve:
// - Se dibujan DOS rectas:
//   1) Recta original: (x0,y0) → (x1,y1).
//   2) Recta trasladada: (x0+ox,y0+oy) → (x1+ox,y1+oy).
// - Ambas se rasterizan con DDA y Bresenham, se dibujan en colores distintos
//   en la imagen PPM, pero SOLO la recta original se compara en comparacion.txt.
//
// Convenciones:
// - Origen del USUARIO: (0,0) abajo-izquierda; y crece hacia ARRIBA.
// - Image::putUser(x,y,RGB) hace el flip vertical al exportar el PPM.
// - Bresenham del TP1: por consigna, NO dibuja horizontales (y0==y1) ni verticales (x0==x1).
// - comparacion.txt: tabla de puntos de la recta original (índice, (x,y) Bresenham vs DDA, igualdad/≠).

#include "./image_ppm.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ----------------------------- tipos/útiles -----------------------------

// Punto 2D - Estructura para representar coordenadas enteras en el plano
struct Pt
{
    int x, y; // Coordenadas x e y del punto
};

// Convierte una cadena de caracteres a long long de forma segura
static inline long long to_ll(const char *s) { return std::atoll(s); }

// Verifica si un valor long long está dentro del rango de un int (32 bits)
static inline bool in_int(long long v) { return v >= INT32_MIN && v <= INT32_MAX; }

// Función auxiliar para agregar un punto al final de un vector de puntos
static inline void push(Pt p, std::vector<Pt> &v) { v.push_back(p); }

// Bounding Box - Estructura para representar un rectángulo delimitador
struct BBox
{
    int xmin, ymin, xmax, ymax; // Coordenadas mínimas y máximas del rectángulo
};
// Calcula el bounding box (rectángulo delimitador) de una línea
// Parámetros: coordenadas de inicio (x0,y0) y fin (x1,y1) de la línea
// Retorna: BBox que contiene completamente la línea
static inline BBox bbox_line(int x0, int y0, int x1, int y1)
{
    return {std::min(x0, x1), std::min(y0, y1), std::max(x0, x1), std::max(y0, y1)};
}
// Expande un bounding box para que contenga otro bounding box
// Parámetros: dos bounding boxes a y b
// Retorna: nuevo BBox que contiene ambos bounding boxes de entrada
static inline BBox expand(const BBox &a, const BBox &b)
{
    return {std::min(a.xmin, b.xmin), std::min(a.ymin, b.ymin),
            std::max(a.xmax, b.xmax), std::max(a.ymax, b.ymax)};
}

// --------------------- Bresenham -----------------------------
bool rasterBresenhamPoints(int x0, int y0, int x1, int y1, std::vector<Pt> &out)
{
    out.clear();
    if (y0 == y1)
        return false; // horizontal excluida
    if (x0 == x1)
        return false; // vertical   excluida

    const int dx = std::abs(x1 - x0);
    const int dy = std::abs(y1 - y0);
    const int sx = (x0 < x1) ? 1 : -1;
    const int sy = (y0 < y1) ? 1 : -1;

    int x = x0, y = y0;

    if (dx >= dy)
    {
        // |m| <= 1 → avanzar en x
        int des_var = 2 * dy - dx;         // D0 = 2Δy - Δx
        const int dd_ady = 2 * dy;         // ΔD si elijo adyacente (E/W)
        const int dd_asup = 2 * (dy - dx); // ΔD si elijo diagonal (ajusto y)
        out.reserve(dx + 1);
        for (int i = 0; i <= dx; ++i)
        {
            push({x, y}, out);
            if (des_var < 0)
                des_var += dd_ady;
            else
            {
                des_var += dd_asup;
                y += sy;
            }
            x += sx;
        }
    }
    else
    {
        // |m| > 1 → avanzar en y
        int des_var = 2 * dx - dy;         // D0' = 2Δx - Δy
        const int dd_ady = 2 * dx;         // ΔD' si elijo adyacente (N/S)
        const int dd_asup = 2 * (dx - dy); // ΔD' si ajusto x
        out.reserve(dy + 1);
        for (int i = 0; i <= dy; ++i)
        {
            push({x, y}, out);
            if (des_var < 0)
                des_var += dd_ady;
            else
            {
                des_var += dd_asup;
                x += sx;
            }
            y += sy;
        }
    }
    return true;
}

// ------------------------------ DDA general ------------------------------
void rasterDDAPoints(int x0, int y0, int x1, int y1, std::vector<Pt> &out)
{
    out.clear();
    const int dx = x1 - x0;
    const int dy = y1 - y0;
    const int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0)
    {
        push({x0, y0}, out);
        return;
    }

    const double incX = static_cast<double>(dx) / steps;
    const double incY = static_cast<double>(dy) / steps;

    double x = static_cast<double>(x0);
    double y = static_cast<double>(y0);

    out.reserve(steps + 1);
    for (int i = 0; i <= steps; ++i)
    {
        push({(int)std::lround(x), (int)std::lround(y)}, out);
        x += incX;
        y += incY;
    }
}

// --------------------------- pintar desde lista --------------------------
static inline void drawFromPoints(Image &img, const std::vector<Pt> &v, const RGB &c)
{
    for (const auto &p : v)
        img.putUser(p.x, p.y, c);
}

// ------------------------ tabla de comparación TXT -----------------------
void writeComparisonTable(const std::vector<Pt> &bres,
                          const std::vector<Pt> &dda,
                          const std::string &path,
                          const std::string &titulo)
{
    std::ofstream f(path); // overwrite (no append)
    if (!f)
    {
        std::cerr << "No se pudo abrir " << path << "\n";
        return;
    }

    const size_t n = std::max(bres.size(), dda.size());
    f << "=============================================================\n";
    f << titulo << "\n";
    f << "Idx |  Bresenham (x,y)  |    DDA (x,y)    | =?\n";
    f << "----+--------------------+-----------------+----\n";

    for (size_t i = 0; i < n; ++i)
    {
        bool hb = (i < bres.size());
        bool hd = (i < dda.size());
        int bx = hb ? bres[i].x : 0, by = hb ? bres[i].y : 0;
        int dx = hd ? dda[i].x : 0, dy = hd ? dda[i].y : 0;
        bool diff = (!hb || !hd) ? true : (bx != dx || by != dy);

        // Formateo con ancho fijo para alineación correcta
        f << std::setw(3) << i << " | ";

        // Columna Bresenham con ancho fijo de 18 caracteres
        if (hb)
        {
            std::string bresStr = "(" + std::to_string(bx) + "," + std::to_string(by) + ")";
            f << std::setw(18) << bresStr;
        }
        else
        {
            f << std::setw(18) << "(—)";
        }
        f << " | ";

        // Columna DDA con ancho fijo de 15 caracteres
        if (hd)
        {
            std::string ddaStr = "(" + std::to_string(dx) + "," + std::to_string(dy) + ")";
            f << std::setw(15) << ddaStr;
        }
        else
        {
            f << std::setw(15) << "(—)";
        }
        f << " | " << (diff ? "≠" : "==") << "\n";
    }

    // Resumen
    size_t equals = 0;
    for (size_t i = 0; i < std::min(bres.size(), dda.size()); ++i)
        if (bres[i].x == dda[i].x && bres[i].y == dda[i].y)
            ++equals;

    f << "----+--------------------+-----------------+----\n";
    f << "Bresenham puntos: " << bres.size()
      << " | DDA puntos: " << dda.size()
      << " | Coincidencias: " << equals << "\n\n";
}

// ================================= main ==================================
int main(int argc, char **argv)
{
    if (argc != 6 && argc != 8 && argc != 10)
    {
        std::cerr << "Uso:\n  " << argv[0] << " x0 y0 x1 y1 salida.ppm [W H] [ox oy]\n"
                  << "Ejemplos:\n"
                  << "  " << argv[0] << " 10 80 200 400 out.ppm 600 400 40 -15\n"
                  << "  " << argv[0] << " 0 0 120 70 out.ppm\n";
        return 1;
    }

    // Parseo coords
    long long X0 = to_ll(argv[1]), Y0 = to_ll(argv[2]);
    long long X1 = to_ll(argv[3]), Y1 = to_ll(argv[4]);
    if (!in_int(X0) || !in_int(Y0) || !in_int(X1) || !in_int(Y1))
    {
        std::cerr << "Error: coordenadas fuera de rango int.\n";
        return 1;
    }
    int x0 = int(X0), y0 = int(Y0), x1 = int(X1), y1 = int(Y1);
    std::string outPath = argv[5];

    // Traslación opcional para la 2ª recta (misma pendiente)
    int ox = 20, oy = 20; // default si no se pasa
    if (argc == 10)
    {
        ox = std::atoi(argv[8]);
        oy = std::atoi(argv[9]);
    }

    // Colores
    RGB bg{255, 255, 255};
    RGB black{0, 0, 0};   // Bresenham (recta 1)
    RGB green{0, 180, 0}; // DDA       (recta 1)
    RGB red{220, 0, 0};   // Bresenham (recta 2)
    RGB blue{0, 90, 220}; // DDA       (recta 2)

    // Canvas
    int W = 0, H = 0;
    if (argc == 8 || argc == 10)
    {
        W = std::max(1, std::atoi(argv[6]));
        H = std::max(1, std::atoi(argv[7]));
    }
    else
    {
        // Auto-lienzo que contenga AMBAS rectas (original y trasladada)
        BBox b1 = bbox_line(x0, y0, x1, y1);
        BBox b2 = bbox_line(x0 + ox, y0 + oy, x1 + ox, y1 + oy);
        BBox bb = expand(b1, b2);
        const int M = 10;
        W = std::max(1, (bb.xmax - bb.xmin + 1) + 2 * M);
        H = std::max(1, (bb.ymax - bb.ymin + 1) + 2 * M);

        // Trasladar las coords para que el bbox arranque en (M,M)
        x0 -= (bb.xmin - M);
        x1 -= (bb.xmin - M);
        y0 -= (bb.ymin - M);
        y1 -= (bb.ymin - M);
    }

    Image img(W, H, bg);

    // --------------------- RECTA ORIGINAL (para comparación) ----------------------
    std::vector<Pt> b1, d1;
    bool okB1 = rasterBresenhamPoints(x0, y0, x1, y1, b1);
    if (!okB1)
        std::cerr << "[Aviso] Bresenham: horizontal/vertical (omitida por consigna)\n";
    rasterDDAPoints(x0, y0, x1, y1, d1);

    // Pintar: DDA en verde, Bresenham en negro
    drawFromPoints(img, d1, green);
    if (okB1)
        drawFromPoints(img, b1, black);

    // Tabla comparativa - Solo la recta original (punto 1 del práctico 2)
    writeComparisonTable(b1, d1, "comparacion.txt",
                         "Comparacion - Recta Original (Bresenham vs DDA)");

    // ------------- SEGUNDA RECTA: MISMA PENDIENTE, distinto inicio ---------
    // Se dibuja la segunda recta pero NO se incluye en la tabla de comparación
    const int x0b = x0 + ox, y0b = y0 + oy;
    const int x1b = x1 + ox, y1b = y1 + oy;

    std::vector<Pt> b2, d2;
    bool okB2 = rasterBresenhamPoints(x0b, y0b, x1b, y1b, b2);
    if (!okB2)
        std::cerr << "[Aviso] Bresenham(2): horizontal/vertical (omitida por consigna)\n";
    rasterDDAPoints(x0b, y0b, x1b, y1b, d2);

    // Pintar: DDA en azul, Bresenham en rojo
    drawFromPoints(img, d2, blue);
    if (okB2)
        drawFromPoints(img, b2, red);

    // NOTA: No se genera tabla comparativa para la segunda recta
    // según indicación del usuario (solo punto 1 del práctico 2)

    // Salida imagen
    img.writePPM(outPath);

    return 0;
}