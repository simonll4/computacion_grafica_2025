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

void write_ellipse_pixels(int xc, int yc, int x, int y)
{
    write_pixel(xc + x, yc + y);
    write_pixel(xc - x, yc + y);
    write_pixel(xc + x, yc - y);
    write_pixel(xc - x, yc - y);
}

void drawEllipseMidpoint(int xc, int yc, int rx, int ry)
{
    int x = 0;
    int y = ry;

    // Precalculos
    int rx2 = rx * rx; // rx^2
    int ry2 = ry * ry; // ry^2
    int tworx2 = 2 * rx2;
    int twory2 = 2 * ry2;

    // Decisión inicial para región 1 (multiplicada por 4 para evitar fracciones)
    int d1 = 4 * ry2 - 4 * rx2 * ry + rx2;

    int dx = 0;          // Acumulador para avances en x
    int dy = tworx2 * y; // Acumulador para avances en y

    // Región 1
    while (dx < dy)
    {
        write_ellipse_pixels(xc, yc, x, y);

        if (d1 < 0)
        {
            // Avance solo en X
            x++;
            dx += twory2;
            d1 += dx + 4 * ry2;
        }
        else
        {
            // Avance en X e Y
            x++;
            y--;
            dx += twory2;
            dy -= tworx2;
            d1 += dx - dy + 4 * ry2;
        }
    }

    // Decisión inicial para región 2
    int d2 = (ry2 * (2 * x + 1) * (2 * x + 1) + rx2 * (2 * y - 2) * (2 * y - 2) - 4 * rx2 * ry2);

    // Región 2
    while (y >= 0)
    {
        write_ellipse_pixels(xc, yc, x, y);

        if (d2 > 0)
        {
            // Avance solo en Y
            y--;
            dy -= tworx2;
            d2 += rx2 - dy;
        }
        else
        {
            // Avance en X e Y
            x++;
            y--;
            dx += twory2;
            dy -= tworx2;
            d2 += dx - dy + rx2;
        }
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
    SDL_Window *window = SDL_CreateWindow("Elipses (Midpoint)",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    drawGrid();
    drawAxes();

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

    int xc, yc, rx, ry, counter = 0;
    while (fscanf(file, "%d %d %d %d", &xc, &yc, &rx, &ry) == 4)
    {
        SDL_Color c = colors[counter % 8];
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        drawEllipseMidpoint(xc, yc, rx, ry);
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
