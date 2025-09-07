#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>

using namespace std;

static const int WIDTH = 800;
static const int HEIGHT = 600;

struct Point
{
    int x, y;
};

struct Edge
{
    int ymax; // coordenada Y máxima del lado
    float x;  // intersección X actual con la scanline
    float dx; // incremento en X por scanline (1/m)
};

// ------------------------------------------------------------------
// Auxiliar: dibujar un píxel (invierte Y para que (0,0) esté abajo)
// ------------------------------------------------------------------
inline void putPixel(SDL_Renderer *r, int x, int y, SDL_Color c)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderDrawPoint(r, x, HEIGHT - 1 - y);
}

// ------------------------------------------------------------------
// Leer polígono desde archivo: primera línea n, luego n líneas "x y"
// ------------------------------------------------------------------
vector<Point> readPolygon(const string &filename)
{
    ifstream in(filename);
    if (!in)
    {
        throw runtime_error("No se pudo abrir el archivo: " + filename);
    }
    int n;
    in >> n;
    vector<Point> poly(n);
    for (int i = 0; i < n; ++i)
    {
        in >> poly[i].x >> poly[i].y;
    }
    return poly;
}

// ------------------------------------------------------------------
// Construir GET (Global Edge Table) bucketed por ymin.
// GET[y] guarda todos los lados cuyo ymin == y
// ------------------------------------------------------------------
vector<vector<Edge>> buildGET(const vector<Point> &poly, int &ymin, int &ymax)
{
    ymin = numeric_limits<int>::max();
    ymax = numeric_limits<int>::min();

    for (const auto &p : poly)
    {
        ymin = min(ymin, p.y);
        ymax = max(ymax, p.y);
    }

    // Clamp a la ventana
    ymin = max(0, ymin);
    ymax = min(HEIGHT - 1, ymax);

    vector<vector<Edge>> GET(HEIGHT); // un bucket por cada y posible

    const int n = static_cast<int>(poly.size());
    for (int i = 0; i < n; ++i)
    {
        Point p1 = poly[i];
        Point p2 = poly[(i + 1) % n]; // cierra el polígono el módulo

        // Ignorar horizontales: no aportan intersecciones únicas
        if (p1.y == p2.y)
            continue;

        // Asegurar p1 como el extremo inferior
        if (p1.y > p2.y)
            swap(p1, p2);

        // Crear edge
        Edge e;
        e.ymax = p2.y;
        e.x = static_cast<float>(p1.x);
        e.dx = static_cast<float>(p2.x - p1.x) / static_cast<float>(p2.y - p1.y);

        // Insertar en bucket correspondiente a ymin
        if (p1.y >= 0 && p1.y < HEIGHT)
        {
            GET[p1.y].push_back(e);
        }
    }

    // (Opcional) Ordenar cada bucket por x inicial para un AET más estable.
    for (auto &bucket : GET)
    {
        sort(bucket.begin(), bucket.end(),
             [](const Edge &a, const Edge &b)
             { return a.x < b.x; });
    }

    return GET;
}

// ------------------------------------------------------------------
// Relleno por Scanline usando GET + AET
// ------------------------------------------------------------------
void scanLineFill(SDL_Renderer *renderer, const vector<Point> &poly, SDL_Color fill)
{
    int ymin, ymax;
    auto GET = buildGET(poly, ymin, ymax);

    vector<Edge> AET; // Active Edge Table

    for (int y = ymin; y <= ymax; ++y)
    {
        // 1) Mover bordes que inician en y (GET[y]) hacia AET
        for (const auto &e : GET[y])
        {
            AET.push_back(e);
            GET[y].clear();
        }

        // 2) Eliminar de AET los bordes que terminan en y (y == ymax)
        AET.erase(
            remove_if(AET.begin(), AET.end(),
                      [y](const Edge &e)
                      { return e.ymax == y; }),
            AET.end());

        if (AET.empty())
            continue;

        // 3) Ordenar AET por x
        sort(AET.begin(), AET.end(),
             [](const Edge &a, const Edge &b)
             { return a.x < b.x; });

        // 4) Rellenar entre pares [x1, x2], [x3, x4], ...
        for (size_t i = 0; i + 1 < AET.size(); i += 2)
        {
            int x1 = static_cast<int>(ceil(AET[i].x));
            int x2 = static_cast<int>(floor(AET[i + 1].x)) - 1;
            if (x1 <= x2)
            {
                for (int x = x1; x <= x2; ++x)
                {
                    putPixel(renderer, x, y, fill);
                }
            }
        }

        // 5) Actualizar X para la próxima scanline: x += dx
        for (auto &e : AET)
        {
            e.x += e.dx;
        }
    }
}

// ------------------------------------------------------------------
// MAIN
// ------------------------------------------------------------------
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Uso: " << argv[0] << " poligono.txt\n";
        return 1;
    }

    vector<Point> poly;
    try
    {
        poly = readPolygon(argv[1]);
    }
    catch (const exception &ex)
    {
        cerr << ex.what() << "\n";
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        cerr << "SDL Error: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Scanline Fill (C++)",
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    SDL_Color fill = {255, 0, 0, 255};
    SDL_Color outline = {0, 0, 0, 255};

    bool running = true;
    SDL_Event ev;
    while (running)
    {
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT)
                running = false;
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Relleno
        scanLineFill(renderer, poly, fill);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
