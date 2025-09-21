/*
 * CGyAV - Guía 2 - Trabajo Práctico 1: Elipse con Orientación Arbitraria
 * 
 * DESCRIPCIÓN:
 * Este programa implementa el dibujo de elipses rotadas usando el algoritmo de punto medio
 * como base (del TP5) y añadiendo transformaciones de rotación y conexión anti-huecos.
 * 
 * COMPILACIÓN:
 * g++ -std=c++17 -O2 tp1_ellipse_rotated.cpp -o tp1_ellipse_rotated
 * 
 * USO:
 * ./tp1_ellipse_rotated cx cy rx ry angleDeg W H salida.ppm [R G B]
 * 
 * PARÁMETROS:
 * cx, cy     : coordenadas del centro de la elipse
 * rx, ry     : semiejes horizontal y vertical (enteros >= 0)
 * angleDeg   : ángulo de rotación en grados (puede ser decimal)
 * W, H       : dimensiones del lienzo (ancho x alto en píxeles)
 * salida.ppm : archivo PPM de salida
 * [R G B]    : color del trazo RGB (0-255, opcional, default: negro)
 * 
 * EJEMPLOS:
 * ./tp1_ellipse_rotated 200 150 100 60 45 400 300 elipse_45.ppm
 * ./tp1_ellipse_rotated 250 200 80 120 30 500 400 elipse_roja.ppm 255 0 0
 * 
 * SISTEMA DE COORDENADAS:
 * - Usuario: (0,0) en esquina inferior-izquierda, Y crece hacia ARRIBA
 * - PPM: filas de arriba hacia abajo → se hace flip vertical automáticamente
 * 
 * ALGORITMO IMPLEMENTADO:
 * 1. Algoritmo de Punto Medio para elipses (basado en TP5)
 * 2. Rotación de todos los puntos muestreados usando matriz de rotación
 * 3. Conexión con Bresenham entre puntos consecutivos para eliminar huecos
 * 4. Manejo de casos degenerados (rx=0 o ry=0 → segmentos rotados)
 * 
 * AUTOR: Basado en la implementación del TP5 con extensiones de rotación
 */

#include <cstdint>
#include <cmath>
#include <vector>
#include <array>
#include <optional>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

// ============================================================================
// SECCIÓN 1: ESTRUCTURAS Y UTILIDADES BÁSICAS
// ============================================================================

/**
 * Estructura para representar un color RGB
 */
struct Color { 
    uint8_t r = 0, g = 0, b = 0; 
};

/**
 * Clase para manejo de imágenes PPM
 * Implementa el sistema de coordenadas de usuario con origen abajo-izquierda
 */
struct Image {
    int W, H;                    // Dimensiones de la imagen
    vector<uint8_t> pix;        // Buffer RGB (3 bytes por píxel)

    /**
     * Constructor: crea imagen con color de fondo
     */
    Image(int W, int H, Color bg = {255,255,255}) : W(W), H(H), pix(W*H*3) {
        clear(bg);
    }
    
    /**
     * Limpia toda la imagen con un color uniforme
     */
    void clear(Color c) {
        for (int i = 0; i < W * H; i++) {
            pix[3*i + 0] = c.r; 
            pix[3*i + 1] = c.g; 
            pix[3*i + 2] = c.b;
        }
    }
    
    /**
     * Coloca un píxel en coordenadas de usuario (0,0 = abajo-izquierda)
     * Realiza automáticamente el flip vertical para formato PPM
     */
    inline void putUser(int x, int y, Color c) {
        if (x < 0 || x >= W || y < 0 || y >= H) return; // Clipping básico
        int row = (H - 1 - y);                          // Flip vertical para PPM
        int idx = (row * W + x) * 3;
        pix[idx + 0] = c.r; 
        pix[idx + 1] = c.g; 
        pix[idx + 2] = c.b;
    }
    
    /**
     * Guarda la imagen en formato PPM binario (P6)
     */
    void writePPM(const string& path) {
        ofstream f(path, ios::binary);
        if (!f) { 
            cerr << "Error: No se pudo abrir " << path << " para escribir.\n"; 
            return; 
        }
        f << "P6\n" << W << " " << H << "\n255\n";
        f.write((char*)pix.data(), pix.size());
    }
};

// Redondeo estable "nearest, half-up" para evitar problemas de half-to-even
// Garantiza resultados consistentes en transformaciones geométricas
static inline int iround(double v) { 
    return (int)std::floor(v + 0.5); 
}

/**
 * Algoritmo de Bresenham para dibujar líneas
 * Conecta dos puntos con una línea de 1 píxel de grosor
 * Usado para unir puntos consecutivos y eliminar huecos en la elipse rotada
 */
static void drawLine(Image& img, int x0, int y0, int x1, int y1, Color c) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    
    while (true) {
        img.putUser(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

/**
 * Estructura para representar un punto 2D entero
 */
struct Pt { 
    int x, y; 
};

// ============================================================================
// SECCIÓN 2: TRANSFORMACIONES DE ROTACIÓN
// ============================================================================

/**
 * Clase para aplicar rotaciones 2D eficientemente
 * Precomputa seno y coseno para evitar cálculos repetidos
 */
struct Rotator {
    double cos_a, sin_a;    // Valores precomputados de la rotación
    int cx, cy;             // Centro de rotación en coordenadas de usuario
    
    /**
     * Constructor: prepara la rotación con ángulo dado
     * @param angleDeg Ángulo de rotación en grados
     * @param cx, cy Centro de rotación
     */
    Rotator(double angleDeg, int cx, int cy) : cx(cx), cy(cy) {
        double a = angleDeg * M_PI / 180.0;  // Convertir a radianes
        cos_a = std::cos(a);
        sin_a = std::sin(a);
    }
    
    /**
     * Aplica la rotación a un punto y redondea establemente
     * Usa la matriz de rotación 2D estándar:
     * [cosθ  -senθ] [x-cx]   [cx]
     * [senθ   cosθ] [y-cy] + [cy]
     * 
     * @param ux, uy Coordenadas del punto a rotar
     * @return Punto rotado con coordenadas enteras
     */
    inline Pt apply(int ux, int uy) const {
        // Trasladar al origen
        double tx = double(ux - cx);
        double ty = double(uy - cy);
        
        // Aplicar rotación
        double rx = tx * cos_a - ty * sin_a;
        double ry = tx * sin_a + ty * cos_a;
        
        // Trasladar de vuelta y redondear
        return Pt{ iround(rx + cx), iround(ry + cy) };
    }
};

// ============================================================================
// SECCIÓN 3: ALGORITMO DE PUNTO MEDIO PARA ELIPSES
// ============================================================================

/**
 * Implementa el algoritmo de punto medio para elipses (basado en TP5)
 * Genera puntos solo del primer cuadrante para luego aplicar simetría
 * 
 * ALGORITMO:
 * - Ecuación de elipse: (x/rx)² + (y/ry)² = 1
 * - Dos regiones según pendiente de la tangente
 * - Región 1: |pendiente| < 1 (avanzar principalmente en x)
 * - Región 2: |pendiente| > 1 (avanzar principalmente en y)
 * - Variables de decisión enteras para evitar aritmética flotante
 */
static vector<Pt> ellipseMidpointFirstQuadrant(int rx, int ry) {
    vector<Pt> quad;
    
    // Validación de parámetros
    if (rx < 0 || ry < 0) return quad;
    if (rx == 0 && ry == 0) { 
        quad.push_back({0, 0}); 
        return quad; 
    }

    // Precalcular cuadrados para evitar multiplicaciones repetidas
    long long rx2 = 1LL * rx * rx;  // rx²
    long long ry2 = 1LL * ry * ry;  // ry²

    // Inicialización: comenzar en (0, ry)
    long long x = 0;
    long long y = ry;

    // Acumuladores incrementales para las derivadas
    long long dx = 2 * ry2 * x;     // 2*ry²*x
    long long dy = 2 * rx2 * y;     // 2*rx²*y

    // ===== REGIÓN 1: AVANZAR PRINCIPALMENTE EN X =====
    // Condición: dx < dy (equivalente a |pendiente| < 1)
    long long d1 = ry2 - rx2 * ry + rx2 / 4;
    
    while (dx < dy) {
        quad.push_back({int(x), int(y)});
        
        if (d1 < 0) {
            // Elegir píxel ESTE (solo incrementar x)
            x++; 
            dx += 2 * ry2;
            d1 += dx + ry2;
        } else {
            // Elegir píxel SURESTE (incrementar x, decrementar y)
            x++; 
            y--; 
            dx += 2 * ry2; 
            dy -= 2 * rx2;
            d1 += dx - dy + ry2;
        }
    }

    // ===== REGIÓN 2: AVANZAR PRINCIPALMENTE EN Y =====
    // Condición: y >= 0 (hasta llegar al eje x)
    long long d2 = ry2 * (x + 0.5) * (x + 0.5) + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
    
    while (y >= 0) {
        quad.push_back({int(x), int(y)});
        
        if (d2 > 0) {
            // Elegir píxel SUR (solo decrementar y)
            y--; 
            dy -= 2 * rx2;
            d2 += rx2 - dy;
        } else {
            // Elegir píxel SURESTE (incrementar x, decrementar y)
            x++; 
            y--; 
            dx += 2 * ry2; 
            dy -= 2 * rx2;
            d2 += dx - dy + rx2;
        }
    }
    
    return quad;
}

// ============================================================================
// SECCIÓN 4: DIBUJO DE ELIPSE ROTADA CON CONEXIÓN ANTI-HUECOS
// ============================================================================

/**
 * Función principal para dibujar una elipse rotada
 * 
 * ALGORITMO:
 * 1. Maneja casos degenerados (rx=0 o ry=0 → segmentos rotados)
 * 2. Genera puntos del primer cuadrante usando algoritmo de punto medio
 * 3. Aplica simetría de 4 puntos y rota cada uno
 * 4. Conecta puntos consecutivos con Bresenham para eliminar huecos
 */
static void drawRotatedEllipseContour(Image& img,
                                      int cx, int cy, int rx, int ry,
                                      double angleDeg, Color col)
{
    // ===== CASOS DEGENERADOS =====
    
    // Punto único (rx = ry = 0)
    if (rx == 0 && ry == 0) { 
        img.putUser(cx, cy, col); 
        return; 
    }
    
    // Segmento vertical (rx = 0, ry > 0)
    if (rx == 0 && ry > 0) {
        Rotator R(angleDeg, cx, cy);
        Pt a = R.apply(cx, cy + ry);    // Extremo superior
        Pt b = R.apply(cx, cy - ry);    // Extremo inferior
        drawLine(img, a.x, a.y, b.x, b.y, col);
        return;
    }
    
    // Segmento horizontal (ry = 0, rx > 0)
    if (ry == 0 && rx > 0) {
        Rotator R(angleDeg, cx, cy);
        Pt a = R.apply(cx + rx, cy);    // Extremo derecho
        Pt b = R.apply(cx - rx, cy);    // Extremo izquierdo
        drawLine(img, a.x, a.y, b.x, b.y, col);
        return;
    }

    // ===== ELIPSE NORMAL =====
    
    // Generar puntos del primer cuadrante usando algoritmo de punto medio
    auto quad = ellipseMidpointFirstQuadrant(rx, ry);
    if (quad.empty()) return;

    // Preparar rotador con el ángulo especificado
    Rotator R(angleDeg, cx, cy);

    // Para cada cuadrante guardamos el punto rotado previo para conectar
    // Índices: 0:(+x,+y)  1:(-x,+y)  2:(+x,-y)  3:(-x,-y)
    array<optional<Pt>, 4> puntoAnterior = { nullopt, nullopt, nullopt, nullopt };

    // Función lambda para procesar un punto con simetría y rotación
    auto procesarPunto = [&](int signoX, int signoY, int cuadrante, int x, int y) {
        // Aplicar simetría y trasladar al centro
        int ux = cx + signoX * x;
        int uy = cy + signoY * y;
        
        // Rotar el punto y redondear establemente
        Pt puntoRotado = R.apply(ux, uy);

        if (puntoAnterior[cuadrante]) {
            // Hay un punto anterior: conectar con línea
            Pt anterior = puntoAnterior[cuadrante].value();
            if (!(anterior.x == puntoRotado.x && anterior.y == puntoRotado.y)) {
                // Puntos diferentes: dibujar línea conectora
                drawLine(img, anterior.x, anterior.y, puntoRotado.x, puntoRotado.y, col);
            } else {
                // Mismo punto: solo dibujar el píxel
                img.putUser(puntoRotado.x, puntoRotado.y, col);
            }
        } else {
            // Primer punto del cuadrante: solo dibujarlo
            img.putUser(puntoRotado.x, puntoRotado.y, col);
        }
        
        // Guardar como punto anterior para la próxima iteración
        puntoAnterior[cuadrante] = puntoRotado;
    };

    // Procesar todos los puntos del primer cuadrante aplicando simetría
    for (size_t i = 0; i < quad.size(); ++i) {
        int x = quad[i].x;
        int y = quad[i].y;
        
        // Aplicar simetría de 4 puntos y rotar cada uno
        procesarPunto(+1, +1, 0, x, y);  // Cuadrante I:  (+x, +y)
        procesarPunto(-1, +1, 1, x, y);  // Cuadrante II: (-x, +y)
        procesarPunto(+1, -1, 2, x, y);  // Cuadrante IV: (+x, -y)
        procesarPunto(-1, -1, 3, x, y);  // Cuadrante III:(-x, -y)
    }

    // NOTA IMPORTANTE: No agregamos "caps" (extremos adicionales)
    // Esto evita píxeles sueltos fuera del contorno de la elipse
}

// ============================================================================
// SECCIÓN 5: FUNCIÓN PRINCIPAL
// ============================================================================

/**
 * Función principal del programa
 * 
 * Parámetros de línea de comandos:
 * 1. cx - Coordenada X del centro de la elipse
 * 2. cy - Coordenada Y del centro de la elipse
 * 3. rx - Semieje horizontal (entero >= 0)
 * 4. ry - Semieje vertical (entero >= 0)
 * 5. angleDeg - Ángulo de rotación en grados (puede ser decimal)
 * 6. W - Ancho de la imagen en píxeles
 * 7. H - Alto de la imagen en píxeles
 * 8. salida.ppm - Nombre del archivo de imagen de salida
 * 9-11. [R G B] - Color del trazo (opcional, por defecto negro)
 */
int main(int argc, char** argv) {
    // Verificar argumentos mínimos
    if (argc < 9) {
        cerr << "=== TP1 - Elipse Rotada ===\n"
             << "Uso: " << argv[0]
             << " cx cy rx ry angleDeg W H salida.ppm [R G B]\n"
             << "\n"
             << "Ejemplos:\n"
             << "  " << argv[0] << " 200 150 100 60 45 400 300 elipse_45.ppm\n"
             << "  " << argv[0] << " 250 200 80 120 30 500 400 elipse_roja.ppm 255 0 0\n";
        return 1;
    }

    // Variables para almacenar parámetros
    int cx, cy, rx, ry, W, H;
    double angleDeg;
    string rutaSalida;
    Color colorTrazo{0, 0, 0};           // Negro por defecto
    const Color colorFondo{255, 255, 255}; // Fondo blanco

    // Función auxiliar para saturar valores RGB
    auto saturarRGB = [](int valor) { 
        return std::max(0, std::min(255, valor)); 
    };

    // Parsear argumentos de línea de comandos
    try {
        cx = stoi(argv[1]);         // Centro X
        cy = stoi(argv[2]);         // Centro Y
        rx = stoi(argv[3]);         // Semieje horizontal
        ry = stoi(argv[4]);         // Semieje vertical
        angleDeg = stod(argv[5]);   // Ángulo de rotación
        W = stoi(argv[6]);          // Ancho de imagen
        H = stoi(argv[7]);          // Alto de imagen
        rutaSalida = argv[8];       // Archivo de salida
        
        // Color opcional del trazo
        if (argc >= 12) {
            colorTrazo.r = (uint8_t)saturarRGB(stoi(argv[9]));
            colorTrazo.g = (uint8_t)saturarRGB(stoi(argv[10]));
            colorTrazo.b = (uint8_t)saturarRGB(stoi(argv[11]));
        }
    } catch (const std::exception& e) {
        cerr << "Error al parsear argumentos: " << e.what() << "\n"
             << "\n"
             << "Uso correcto: " << argv[0]
             << " cx cy rx ry angleDeg W H salida.ppm [R G B]\n";
        return 1;
    }

    // Validar parámetros
    if (W <= 0 || H <= 0) { 
        cerr << "Error: Las dimensiones W y H deben ser mayores que 0.\n"
             << "Valores recibidos: W=" << W << ", H=" << H << "\n";
        return 1; 
    }
    if (rx < 0 || ry < 0) { 
        cerr << "Error: Los semiejes rx y ry deben ser >= 0.\n"
             << "Valores recibidos: rx=" << rx << ", ry=" << ry << "\n";
        return 1; 
    }

    // Crear imagen y dibujar elipse rotada
    Image imagen(W, H, colorFondo);
    drawRotatedEllipseContour(imagen, cx, cy, rx, ry, angleDeg, colorTrazo);
    
    // Guardar imagen resultante
    imagen.writePPM(rutaSalida);

    // Mostrar resumen de la operación
    cout << "\n=== TP1 - Elipse Rotada Completada ===\n"
         << "Centro: (" << cx << ", " << cy << ")\n"
         << "Semiejes: rx=" << rx << ", ry=" << ry << "\n"
         << "Ángulo de rotación: " << angleDeg << "°\n"
         << "Dimensiones imagen: " << W << " x " << H << " píxeles\n"
         << "Color de trazo: RGB(" << (int)colorTrazo.r << ", " 
                                  << (int)colorTrazo.g << ", " 
                                  << (int)colorTrazo.b << ")\n"
         << "Archivo generado: " << rutaSalida << "\n"
         << "\n"
         << "La elipse ha sido dibujada exitosamente con rotación aplicada.\n";

    return 0;
}