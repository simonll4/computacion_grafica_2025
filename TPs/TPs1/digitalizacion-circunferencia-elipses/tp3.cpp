// CGyAV - Práctico 3: Digitalización de Circunferencias con Algoritmo de Punto Medio
//
// COMPILACIÓN:
//   g++ -std=c++17 -O2 tp3.cpp -o tp3
//
// USO:
//   ./tp3 salida.ppm W H cx cy r [R G B]
//
// PARÁMETROS:
//   salida.ppm : archivo PPM de salida
//   W H        : dimensiones del lienzo (ancho x alto en píxeles)
//   cx cy      : coordenadas del centro de la circunferencia
//   r          : radio de la circunferencia (entero >= 0)
//   [R G B]    : color del trazo RGB (0-255, opcional, default: negro)
//
// EJEMPLOS DE EJECUCIÓN:
//   ./tp3 circulo.ppm 400 300 200 150 80
//   ./tp3 circulo_rojo.ppm 500 400 250 200 100 255 0 0
//   ./tp3 circulo_azul.ppm 600 600 300 300 150 0 0 255
//
// SISTEMA DE COORDENADAS:
//   - Origen: (0,0) en esquina inferior-izquierda
//   - Eje X: hacia la derecha
//   - Eje Y: hacia arriba
//   - La función putUser() maneja automáticamente el flip vertical
//
// ALGORITMO DE PUNTO MEDIO (BRESENHAM PARA CÍRCULOS):
//   - Calcula solo el segundo octante (0° a 45°)
//   - Usa simetría de 8 puntos para completar la circunferencia
//   - Variable de decisión d determina el siguiente píxel a pintar
//   - Inicialización: x=0, y=r, d=1-r
//   - Bucle mientras x <= y:
//     * Si d < 0:  elegir píxel ESTE     → d += 2x + 3,     x++
//     * Si d >= 0: elegir píxel SURESTE → d += 2(x-y) + 5, x++, y--

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string>
#include "../common/image_ppm.h"

static inline RGB rgbClamp(int r, int g, int b) {
    auto sat = [](int v){ return (v < 0 ? 0 : (v > 255 ? 255 : v)); };
    return RGB(uint8_t(sat(r)), uint8_t(sat(g)), uint8_t(sat(b)));
}

// ===== FUNCIÓN DE SIMETRÍA: PINTAR 8 PUNTOS =====
static inline void plot8(Image &img, int cx, int cy, int x, int y, RGB col) {
    // Cuadrante I (superior derecho)
    img.putUser(cx + x, cy + y, col);          // Octante 2: (+x, +y)
    img.putUser(cx + y, cy + x, col);          // Octante 1: (+y, +x)
    
    // Cuadrante II (superior izquierdo)  
    img.putUser(cx - x, cy + y, col);          // Octante 3: (-x, +y)
    img.putUser(cx - y, cy + x, col);          // Octante 4: (-y, +x)
    
    // Cuadrante III (inferior izquierdo)
    img.putUser(cx - x, cy - y, col);          // Octante 6: (-x, -y)
    img.putUser(cx - y, cy - x, col);          // Octante 7: (-y, -x)
    
    // Cuadrante IV (inferior derecho)
    img.putUser(cx + x, cy - y, col);          // Octante 5: (+x, -y)
    img.putUser(cx + y, cy - x, col);          // Octante 8: (+y, -x)
}

// ===== ALGORITMO DE PUNTO MEDIO PARA CIRCUNFERENCIAS (BRESENHAM) =====
// 
// TEORÍA:
// - Ecuación del círculo: (x-cx)² + (y-cy)² = r²
// - Función implícita: F(x,y) = (x-cx)² + (y-cy)² - r²
//   * F(x,y) = 0: punto SOBRE la circunferencia
//   * F(x,y) < 0: punto DENTRO de la circunferencia  
//   * F(x,y) > 0: punto FUERA de la circunferencia
//
// ESTRATEGIA:
// - Calcular solo el segundo octante (0° a 45°, donde x ≤ y)
// - Usar simetría para obtener los otros 7 octantes
// - En cada paso, elegir entre píxel ESTE (E) o SURESTE (SE)
// - Variable de decisión d evalúa el punto medio entre E y SE
//
// FÓRMULAS:
// - d₀ = 1 - r (valor inicial)
// - Si d < 0: elegir E, actualizar d += 2x + 3
// - Si d ≥ 0: elegir SE, actualizar d += 2(x-y) + 5
//
void drawCircleMidpoint(Image &img, int cx, int cy, int r, RGB col) {
    // Casos especiales
    if (r < 0) return;                          // Radio inválido
    if (r == 0) { 
        img.putUser(cx, cy, col); 
        return; 
    }                                           // Punto único en el centro

    // ===== INICIALIZACIÓN DEL ALGORITMO DE PUNTO MEDIO =====
    int x = 0;                                  // Comenzamos en x = 0
    int y = r;                                  // y = radio (punto más alto)
    int d = 1 - r;                              // Variable de decisión inicial: d₀ = 1 - r
    
    // Pintar el punto inicial (0, r) y sus 8 simetrías
    plot8(img, cx, cy, x, y, col);

    // ===== BUCLE PRINCIPAL: RECORRER EL SEGUNDO OCTANTE =====
    // Condición: x <= y (hasta que crucemos la diagonal de 45°)
    while (x <= y) {
        
        // ===== EVALUACIÓN DE LA VARIABLE DE DECISIÓN =====
        if (d < 0) {
            // CASO 1: d < 0 → El punto medio está DENTRO del círculo
            // Elegimos el píxel ESTE (E): avanzamos solo en x
            d += (2 * x + 3);                  // Δd_E = 2x + 3
            x += 1;                            // Solo incrementamos x
            
        } else {
            // CASO 2: d >= 0 → El punto medio está FUERA del círculo  
            // Elegimos el píxel SURESTE (SE): avanzamos en x y decrementamos y
            d += (2 * (x - y) + 5);            // Δd_SE = 2(x - y) + 5
            x += 1;                            // Incrementamos x
            y -= 1;                            // Decrementamos y
        }
        
        // ===== PINTAR LAS 8 SIMETRÍAS DEL NUEVO PUNTO =====
        // Cada punto (x,y) calculado se refleja en los 8 octantes
        plot8(img, cx, cy, x, y, col);
    }
    
    // Al finalizar: hemos trazado completamente la circunferencia
    // usando solo aritmética entera y evaluaciones simples de la variable d
}

int main(int argc, char** argv) {
    if (argc < 7) {
        std::cerr
            << "Uso: ./tp3 salida.ppm W H cx cy r [R G B]\n"
            << "Ej.: ./tp3 circ.ppm 400 300 200 150 80  0 0 0\n";
        return 1;
    }

    const std::string outPath = argv[1];
    const int W  = std::atoi(argv[2]);
    const int H  = std::atoi(argv[3]);
    const int cx = std::atoi(argv[4]);
    const int cy = std::atoi(argv[5]);
    const int r  = std::atoi(argv[6]);

    if (W <= 0 || H <= 0) {
        std::cerr << "Error: W y H deben ser > 0.\n";
        return 1;
    }
    if (r < 0) {
        std::cerr << "Error: el radio r debe ser >= 0.\n";
        return 1;
    }

    // Color de trazo opcional (default negro)
    RGB stroke = rgbClamp(0,0,0);
    if (argc >= 10) {
        stroke = rgbClamp(std::atoi(argv[7]), std::atoi(argv[8]), std::atoi(argv[9]));
    }

    // Fondo fijo: blanco
    const RGB bg = rgbClamp(255,255,255);

    // Crear imagen y dibujar circunferencia completa
    Image img(W, H, bg);
    drawCircleMidpoint(img, cx, cy, r, stroke);

    // Escribir PPM
    img.writePPM(outPath);

    // Resumen en stdout
    std::cout << "---- Circunferencia (Punto Medio) ----\n"
              << "Centro: (" << cx << "," << cy << "), r=" << r << "\n"
              << "Lienzo: " << W << "x" << H << "\n"
              << "Trazo: (" << int(stroke.r) << "," << int(stroke.g) << "," << int(stroke.b) << ")\n"
              << "Fondo: (" << int(bg.r) << "," << int(bg.g) << "," << int(bg.b) << ")\n"
              << "Salida: " << outPath << "\n";
    return 0;
}
