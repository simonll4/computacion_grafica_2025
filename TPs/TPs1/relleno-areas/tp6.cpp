// CGyAV - Práctico 6: Relleno de polígonos por Scanline (GET/AET) leyendo tabla de archivo
// Compilar: g++ -std=c++17 -O2 tp6.cpp -o tp6
//
// Uso:
//   ./tp6 salida.ppm  W  H  vertices.txt  [R G B] --> ./tp6 figura_red.ppm 120 80 vertices_practico6.txt  255 0 0
//
// - Lee vértices desde un archivo de texto con el formato de tabla del práctico:
//       # x y
//       1 10 10
//       2 10 50
//       3 40 20
//       4 40 40
//       5 70 10
//   Reglas del parser:
//     * Líneas que empiezan con '#' se ignoran (comentarios/encabezado).
//     * Cada fila puede ser: "id x y" (tres enteros) o "x y" (dos enteros).
//     * Separadores: espacios o tabs (también soporta comas).
//   El polígono se cierra implícitamente (último vértice conecta al primero).
//
// - Implementa Scanline optimizado:
//     * GET (Global Edge Table) por buckets de y_min
//     * AET (Active Edge Table), regla even-odd, ordenada por x actual
//   Reglas robustas:
//     * Ignorar aristas horizontales (y1 == y2)
//     * Convención top-inclusive / bottom-exclusive: arista activa en y = [ceil(ymin), ceil(ymax)).
//     * x inicia en la intersección exacta con y_min, se actualiza con invSlope por scanline.
//
// el archivo vertices_practico6.txt  es la tabla de la guía

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

#include "../common/image_ppm.h"

static inline RGB rgbClamp(int r, int g, int b)
{
    auto sat = [](int v)
    { return (v < 0 ? 0 : (v > 255 ? 255 : v)); };
    return RGB(uint8_t(sat(r)), uint8_t(sat(g)), uint8_t(sat(b)));
}

// ----------------- Parser de tabla -----------------

// Intenta parsear una línea con "id x y" o "x y" (soporta comas)
static bool parseVertexLine(const std::string &line, std::pair<int, int> &out)
{
    // Quitar comentarios # desde el inicio
    std::string s = line;
    // Si empieza con '#', descartar directamente
    if (!s.empty() && s[0] == '#')
        return false;

    // Reemplazar comas por espacios para simplificar
    for (char &c : s)
        if (c == ',')
            c = ' ';

    std::istringstream iss(s);
    std::vector<long long> vals;
    long long v;
    while (iss >> v)
        vals.push_back(v);
    if (vals.empty())
        return false;

    if (vals.size() >= 3)
    {
        // id x y
        out.first = int(vals[1]);
        out.second = int(vals[2]);
        return true;
    }
    else if (vals.size() == 2)
    {
        // x y
        out.first = int(vals[0]);
        out.second = int(vals[1]);
        return true;
    }
    return false;
}

static bool loadVerticesFile(const std::string &path, std::vector<std::pair<int, int>> &vertices)
{
    std::ifstream in(path);
    if (!in)
        return false;
    std::string line;
    while (std::getline(in, line))
    {
        std::pair<int, int> p;
        if (parseVertexLine(line, p))
            vertices.push_back(p);
    }
    return vertices.size() >= 3;
}

// ----------------- Estructuras GET/AET -----------------

struct Edge
{
    int yMax;        // top-exclusive
    double x;        // x actual en la scanline y
    double invSlope; // dx/dy
};

static std::vector<std::vector<Edge>> buildGET(const std::vector<std::pair<int, int>> &vtx, int &yMinOut, int &yMaxOut)
{
    std::vector<std::vector<Edge>> GET;
    yMinOut = INT_MAX;
    yMaxOut = INT_MIN;

    const int n = (int)vtx.size();
    if (n < 3)
        return GET;

    for (auto [x, y] : vtx)
    {
        yMinOut = std::min(yMinOut, y);
        yMaxOut = std::max(yMaxOut, y);
    }
    if (yMinOut == INT_MAX)
        yMinOut = 0;
    if (yMaxOut == INT_MIN)
        yMaxOut = 0;

    GET.assign(std::max(0, yMaxOut - yMinOut + 1), {});

    auto addEdge = [&](int x1, int y1, int x2, int y2)
    {
        if (y1 == y2)
            return; // ignorar horizontales
        if (y1 > y2)
        {
            std::swap(y1, y2);
            std::swap(x1, x2);
        }

        const double dy = double(y2 - y1);
        const double dx = double(x2 - x1);
        const double invSlope = dx / dy;

        int y_min = int(std::ceil(y1));
        int y_max = int(std::ceil(y2));
        if (y_min == y_max)
            return;

        double x_at_y_min = double(x1) + invSlope * (double(y_min) - double(y1));

        int bucket = y_min - yMinOut;
        if (bucket < 0 || bucket >= (int)GET.size())
            return;
        GET[bucket].push_back(Edge{y_max, x_at_y_min, invSlope});
    };

    for (int i = 0; i < n; ++i)
    {
        auto [x1, y1] = vtx[i];
        auto [x2, y2] = vtx[(i + 1) % n];
        addEdge(x1, y1, x2, y2);
    }
    return GET;
}

static inline void fillSpan(Image &img, int y, int x0, int x1, const RGB &col)
{
    if (x0 > x1)
        std::swap(x0, x1);
    for (int x = x0; x <= x1; ++x)
        img.putUser(x, y, col);
}

static void scanlineFillPolygon(Image &img, const std::vector<std::pair<int, int>> &vtx, const RGB &fillColor)
{
    if (vtx.size() < 3)
        return;

    int globalYMin, globalYMax;
    auto GET = buildGET(vtx, globalYMin, globalYMax);
    if (GET.empty())
        return;

    std::vector<Edge> AET;

    for (int y = globalYMin; y < globalYMax; ++y)
    {
        int bucketIdx = y - globalYMin;

        if (bucketIdx >= 0 && bucketIdx < (int)GET.size())
        {
            for (const auto &e : GET[bucketIdx])
                AET.push_back(e);
        }

        AET.erase(std::remove_if(AET.begin(), AET.end(),
                                 [&](const Edge &e)
                                 { return e.yMax <= y; }),
                  AET.end());

        if (AET.empty())
            continue;

        std::sort(AET.begin(), AET.end(),
                  [](const Edge &a, const Edge &b)
                  {
                      if (a.x != b.x)
                          return a.x < b.x;
                      return a.yMax < b.yMax;
                  });

        for (size_t i = 0; i + 1 < AET.size(); i += 2)
        {
            double xL = AET[i].x;
            double xR = AET[i + 1].x;
            int x0 = (int)std::ceil(xL);
            int x1 = (int)std::floor(xR - 1e-9);
            if (x0 <= x1)
                fillSpan(img, y, x0, x1, fillColor);
        }

        for (auto &e : AET)
            e.x += e.invSlope;
    }
}

// ----------------- Main -----------------

int main(int argc, char **argv)
{
    if (argc < 5)
    {
        std::cerr
            << "Uso: ./tp6 salida.ppm W H vertices.txt [R G B]\n"
            << "Ej.: ./tp6 poly.ppm 120 80 vertices_practico6.txt   0 0 0\n";
        return 1;
    }

    const std::string outPath = argv[1];
    const int W = std::atoi(argv[2]);
    const int H = std::atoi(argv[3]);
    const std::string verticesPath = argv[4];

    if (W <= 0 || H <= 0)
    {
        std::cerr << "Error: W y H deben ser > 0.\n";
        return 1;
    }

    RGB fill = rgbClamp(0, 0, 0);
    if (argc >= 8)
        fill = rgbClamp(std::atoi(argv[5]), std::atoi(argv[6]), std::atoi(argv[7]));
    const RGB bg = rgbClamp(255, 255, 255);

    std::vector<std::pair<int, int>> poly;
    if (!loadVerticesFile(verticesPath, poly))
    {
        std::cerr << "Error: no pude leer vértices desde '" << verticesPath
                  << "' (necesito >= 3 vertices). Formato esperado: lineas 'id x y' o 'x y'.\n";
        return 1;
    }

    Image img(W, H, bg);
    scanlineFillPolygon(img, poly, fill);
    img.writePPM(outPath);

    std::cout << "---- Scanline Fill (Práctico 6) ----\n"
              << "Vertices: " << poly.size() << "  (archivo=" << verticesPath << ")\n"
              << "Lienzo: " << W << "x" << H << "\n"
              << "Relleno: (" << int(fill.r) << "," << int(fill.g) << "," << int(fill.b) << ")\n"
              << "Salida: " << outPath << "\n";
    return 0;
}
