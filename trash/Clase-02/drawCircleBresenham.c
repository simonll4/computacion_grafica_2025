#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_WIDTH 544
#define SCREEN_HEIGHT 544
#define GRID_SIZE 32

SDL_Renderer *renderer;

void drawGrid()
{
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    for (int x = 0; x <= SCREEN_WIDTH; x += GRID_SIZE)
        SDL_RenderDrawLine(renderer, x, 0, x, SCREEN_HEIGHT);
    for (int y = 0; y <= SCREEN_HEIGHT; y += GRID_SIZE)
        SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
}

void drawAxes()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    int midX = SCREEN_WIDTH / 2;
    int midY = SCREEN_HEIGHT / 2;
    SDL_RenderDrawLine(renderer, midX, 0, midX, SCREEN_HEIGHT);
    SDL_RenderDrawLine(renderer, 0, midY, SCREEN_WIDTH, midY);
}

void write_pixel(int x, int y)
{
    int cells_x = SCREEN_WIDTH / GRID_SIZE;
    int cells_y = SCREEN_HEIGHT / GRID_SIZE;
    int center_cell_x = cells_x / 2;
    int center_cell_y = cells_y / 2;

    int cell_x = center_cell_x + x;
    int cell_y = center_cell_y - y;

    if (cell_x >= 0 && cell_x < cells_x && cell_y >= 0 && cell_y < cells_y)
    {
        // if (x == 0 && y == 0) return;

        int pixel_x = cell_x * GRID_SIZE;
        int pixel_y = cell_y * GRID_SIZE;
        int margin = 4;
        SDL_Rect rect = {
            pixel_x + margin,
            pixel_y + margin,
            GRID_SIZE - 2 * margin,
            GRID_SIZE - 2 * margin};
        SDL_RenderFillRect(renderer, &rect);
    }
}

// Aplica las 8 reflexiones desde (x,y) calculado en el segundo octante
// xc,yc = centro de la circunferencia
void write_circle_pixels(int xc, int yc, int x, int y)
{
    // 1) Punto original
    write_pixel(xc + x, yc + y);

    // 2) Reflejo eje Y
    write_pixel(xc - x, yc + y);

    // 3) Reflejo eje X
    write_pixel(xc + x, yc - y);

    // 4) Reflejo ambos ejes
    write_pixel(xc - x, yc - y);

    // 5) Reflejo diagonal
    write_pixel(xc + y, yc + x);

    // 6) Diagonal + eje Y
    write_pixel(xc - y, yc + x);

    // 7) Diagonal + eje X
    write_pixel(xc + y, yc - x);

    // 8) Diagonal + ambos ejes
    write_pixel(xc - y, yc - x);
}

void drawCircleMP(int xc, int yc, int r)
{
    int des_var = 5 - 4 * r;
    int dd_ady = 12;
    int dd_ainf = 20 - 8 * r;
    int x = 0;
    int y = r;

    write_circle_pixels(xc, yc, x, y);
    x++;

    while (y > x)
    {
        if (des_var < 0)
        {
            des_var += dd_ady;
        }
        else
        {
            des_var += dd_ainf;
            y--;
        }

        // Dk = 4 * (2 (x + 1) + 1)
        dd_ady = 8 * x + 12;
        // D’k = 4 * (2 * (x + 1) - 2 * y + 3)
        dd_ainf = 8 * x - 8 * y + 20;

        write_circle_pixels(xc, yc, x, y);
        x++;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Uso: %s archivo_entrada.txt\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file)
    {
        perror("Error abriendo archivo");
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Circunferencias (Midpoint)",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Fondo blanco
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    drawGrid();
    drawAxes();

    // Colores para las circunferencias
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

    int x, y, r, counter = 0;
    while (fscanf(file, "%d %d %d", &x, &y, &r) == 3)
    {
        SDL_Color c = colors[counter % 8];
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        drawCircleMP(x, y, r);
        counter++;
    }

    fclose(file);

    SDL_RenderPresent(renderer);

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
