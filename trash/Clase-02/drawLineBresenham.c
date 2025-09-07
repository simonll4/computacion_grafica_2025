#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>

//(17 * 32 = 544) → 17 celdas, con la celda número 8 (índice 0 a 16) como centro.
#define SCREEN_WIDTH 544
#define SCREEN_HEIGHT 544
#define GRID_SIZE 32

SDL_Renderer *renderer;

// Dibujar cuadrícula
void drawGrid()
{
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    for (int x = 0; x <= SCREEN_WIDTH; x += GRID_SIZE)
        SDL_RenderDrawLine(renderer, x, 0, x, SCREEN_HEIGHT);
    for (int y = 0; y <= SCREEN_HEIGHT; y += GRID_SIZE)
        SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
}

// Dibujar ejes X e Y
void drawAxes()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    int midX = (SCREEN_WIDTH / 2);
    int midY = (SCREEN_HEIGHT / 2);
    SDL_RenderDrawLine(renderer, midX, 0, midX, SCREEN_HEIGHT);
    SDL_RenderDrawLine(renderer, 0, midY, SCREEN_WIDTH, midY);
}

// Escribir un píxel en la cuadrícula
void write_pixel(int x, int y)
{
    int cells_x = SCREEN_WIDTH / GRID_SIZE;
    int cells_y = SCREEN_HEIGHT / GRID_SIZE;

    int center_cell_x = cells_x / 2; // centro exacto (solo si celdas es impar)
    int center_cell_y = cells_y / 2;

    int cell_x = center_cell_x + x;
    int cell_y = center_cell_y - y;

    if (cell_x >= 0 && cell_x < cells_x && cell_y >= 0 && cell_y < cells_y)
    {
        int pixel_x = cell_x * GRID_SIZE;
        int pixel_y = cell_y * GRID_SIZE;

        int margin = 4;
        SDL_Rect rect = {
            pixel_x + margin,
            pixel_y + margin,
            GRID_SIZE - 2 * margin,
            GRID_SIZE - 2 * margin};

        if (x != 0 || y != 0) // Evitar pintar el origen
            SDL_RenderFillRect(renderer, &rect);
    }
}

// Dibujar una línea usando el algoritmo de Bresenham
// (x0, y0) → punto inicial, (x1, y1) → punto final
void drawLineBresenham(int x0, int y0, int x1, int y1)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1; // Sentido en x
    int sy = (y0 < y1) ? 1 : -1; // Sentido en y

    int x = x0;
    int y = y0;

    // Decide si se avanza principalmente en x o en y
    if (dx >= dy)
    {
        // Pendiente <= 1 → se avanza en x
        int des_var = 2 * dy - dx;
        const int dd_ady = 2 * dy;
        const int dd_asup = 2 * (dy - dx);
        const int steps = dx;

        for (int i = 0; i <= steps; i++)
        {
            write_pixel(x, y);
            if (des_var < 0)
            {
                des_var += dd_ady;
            }
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
        // Pendiente > 1 → se avanza en y
        int des_var = 2 * dx - dy;
        const int dd_ady = 2 * dx;
        const int dd_asup = 2 * (dx - dy);
        const int steps = dy;

        for (int i = 0; i <= steps; i++)
        {
            write_pixel(x, y);
            if (des_var < 0)
            {
                des_var += dd_ady;
            }
            else
            {
                des_var += dd_asup;
                x += sx;
            }
            y += sy;
        }
    }
}

int main(int argc, char *argv[])
{
    // Verificar que se pasó el nombre del archivo como argumento
    if (argc < 2)
    {
        printf("Se debe especificar el archivo coordenadas.\n");
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        perror("Error al abrir el archivo");
        return 1;
    }

    // Inicializar SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Bresenham lineas",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Fondo blanco
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Dibujar cuadrícula y ejes
    drawGrid();
    drawAxes();

    // Definir colores para cada línea (8 colores diferentes)
    SDL_Color colors[8] = {
        {255, 0, 0, 255},   // Rojo
        {0, 0, 255, 255},   // Azul
        {255, 165, 0, 255}, // Naranja
        {0, 255, 0, 255},   // Verde
        {255, 0, 255, 255}, // Magenta
        {255, 255, 0, 255}, // Amarillo
        {128, 0, 128, 255}, // Púrpura
        {0, 128, 128, 255}  // Cian
    };

    // Leer líneas del archivo
    int x0, y0, x1, y1;
    int counter = 0;
    while (fscanf(f, "%d %d %d %d", &x0, &y0, &x1, &y1) == 4)
    {
        // Definir color para esta línea
        SDL_Color currentColor = colors[counter % 8];

        // Configurar color para los píxeles de Bresenham
        SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);

        // Dibujar píxeles con Bresenham
        drawLineBresenham(x0, y0, x1, y1);
        counter++;
    }

    fclose(f);

    if (counter == 0)
    {
        printf("No se dibujaron líneas. Verificá que el archivo tenga coordenadas válidas.\n");
    }
    else
    {
        printf("Se dibujaron %d líneas.\n", counter);
    }

    SDL_RenderPresent(renderer);

    // Esperar hasta que se cierre la ventana
    SDL_Event e;
    int quit = 0;
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                quit = 1;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
