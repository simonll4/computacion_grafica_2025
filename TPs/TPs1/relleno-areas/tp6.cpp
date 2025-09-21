/*
 * CGyAV - Trabajo Práctico 6: Relleno de Polígonos por Algoritmo Scanline
 * 
 * DESCRIPCIÓN:
 * Este programa implementa el algoritmo de relleno de polígonos por scanline usando
 * las estructuras GET (Global Edge Table) y AET (Active Edge Table).
 * 
 * COMPILACIÓN:
 * g++ -std=c++17 -O2 tp6.cpp -o tp6
 * 
 * USO:
 * ./tp6 salida.ppm W H vertices.txt [R G B]
 * 
 * EJEMPLO:
 * ./tp6 poligono_rojo.ppm 120 80 vertices_practico6.txt 255 0 0
 * 
 * FORMATO DEL ARCHIVO DE VÉRTICES:
 * El archivo debe contener una tabla con el formato:
 *   # Comentarios (líneas que empiezan con '#' se ignoran)
 *   1 10 10    # id x y (formato con 3 columnas)
 *   2 10 50    # o simplemente x y (formato con 2 columnas)
 *   3 40 20
 *   4 40 40
 *   5 70 10
 * 
 * ALGORITMO SCANLINE:
 * 1. GET: Tabla global de aristas organizadas por y_mínimo
 * 2. AET: Tabla activa de aristas para cada línea de barrido
 * 3. Relleno: Regla par-impar entre intersecciones ordenadas
 * 
 * AUTOR: Implementación basada en el ejemplo de scanline2.cpp
 */

// Librerías estándar de C++
#include <cstdint>     // Para tipos enteros específicos
#include <cstdlib>     // Para atoi, etc.
#include <cstring>     // Para manejo de strings C
#include <climits>     // Para INT_MAX, INT_MIN
#include <cmath>       // Para ceil, floor
#include <fstream>     // Para lectura de archivos
#include <iostream>    // Para entrada/salida estándar
#include <sstream>     // Para stringstream
#include <string>      // Para std::string
#include <utility>     // Para std::pair
#include <vector>      // Para std::vector
#include <algorithm>   // Para sort, remove_if, etc.

// Librería local para manejo de imágenes PPM
#include "../common/image_ppm.h"

/**
 * Función auxiliar para saturar valores RGB en el rango [0, 255]
 * @param r Componente rojo
 * @param g Componente verde  
 * @param b Componente azul
 * @return Color RGB con valores saturados
 */
static inline RGB rgbClamp(int r, int g, int b)
{
    auto saturar = [](int valor) -> int {
        return (valor < 0) ? 0 : (valor > 255) ? 255 : valor;
    };
    return RGB(uint8_t(saturar(r)), uint8_t(saturar(g)), uint8_t(saturar(b)));
}

// ============================================================================
// SECCIÓN 1: PARSER DE ARCHIVO DE VÉRTICES
// ============================================================================

/**
 * Parsea una línea del archivo de vértices
 * Formatos soportados:
 * - "id x y" (tres enteros)
 * - "x y" (dos enteros)
 * - Soporta comas como separadores además de espacios
 * - Ignora líneas que empiezan con '#'
 */
static bool parseVertexLine(const std::string &line, std::pair<int, int> &out)
{
    // Crear copia de la línea para procesamiento
    std::string s = line;
    
    // Ignorar líneas de comentario (empiezan con '#')
    if (!s.empty() && s[0] == '#')
        return false;

    // Normalizar separadores: convertir comas a espacios
    for (char &c : s) {
        if (c == ',') {
            c = ' ';
        }
    }

    // Extraer todos los números de la línea
    std::istringstream iss(s);
    std::vector<long long> valores;
    long long numero;
    while (iss >> numero) {
        valores.push_back(numero);
    }
    
    // Verificar que hay al menos algún número
    if (valores.empty()) {
        return false;
    }

    // Interpretar según la cantidad de valores
    if (valores.size() >= 3) {
        // Formato: "id x y" -> tomar x e y (índices 1 y 2)
        out.first = int(valores[1]);   // coordenada x
        out.second = int(valores[2]);  // coordenada y
        return true;
    }
    else if (valores.size() == 2) {
        // Formato: "x y" -> tomar x e y directamente
        out.first = int(valores[0]);   // coordenada x
        out.second = int(valores[1]);  // coordenada y
        return true;
    }
    
    // Línea no válida
    return false;
}

/**
 * Carga los vértices desde un archivo de texto
 */
static bool loadVerticesFile(const std::string &path, std::vector<std::pair<int, int>> &vertices)
{
    std::ifstream archivo(path);
    if (!archivo) {
        return false;  // No se pudo abrir el archivo
    }
    
    std::string linea;
    while (std::getline(archivo, linea)) {
        std::pair<int, int> vertice;
        if (parseVertexLine(linea, vertice)) {
            vertices.push_back(vertice);
        }
    }
    
    // Un polígono necesita al menos 3 vértices
    return vertices.size() >= 3;
}

// ============================================================================
// SECCIÓN 2: ESTRUCTURAS Y ALGORITMO SCANLINE
// ============================================================================

/**
 * Estructura que representa una arista en las tablas GET/AET
 */
struct Edge
{
    int yMax;        // Coordenada Y máxima (límite superior exclusivo)
    double x;        // Coordenada X actual en la línea de barrido
    double invSlope; // Pendiente inversa (dx/dy) para actualizar x
};

/**
 * Construye la GET (Global Edge Table) a partir de los vértices del polígono
 * 
 * La GET organiza todas las aristas del polígono en buckets según su y_mínimo.
 * Cada bucket GET[i] contiene las aristas que empiezan en la línea y = yMin + i.
 * 
 * @param vtx Vector de vértices del polígono
 * @param yMinOut Coordenada Y mínima del polígono (salida)
 * @param yMaxOut Coordenada Y máxima del polígono (salida)
 * @return Vector de buckets, donde GET[i] contiene aristas que empiezan en y = yMin + i
 */
static std::vector<std::vector<Edge>> buildGET(const std::vector<std::pair<int, int>> &vtx, int &yMinOut, int &yMaxOut)
{
    std::vector<std::vector<Edge>> GET;
    yMinOut = INT_MAX;
    yMaxOut = INT_MIN;

    const int numVertices = (int)vtx.size();
    if (numVertices < 3) {
        return GET;  // Polígono inválido
    }

    // Encontrar los límites Y del polígono
    for (const auto &[x, y] : vtx) {
        yMinOut = std::min(yMinOut, y);
        yMaxOut = std::max(yMaxOut, y);
    }
    
    // Validar límites
    if (yMinOut == INT_MAX) yMinOut = 0;
    if (yMaxOut == INT_MIN) yMaxOut = 0;

    // Crear buckets para cada línea Y posible
    GET.assign(std::max(0, yMaxOut - yMinOut + 1), {});

    // Función lambda para agregar una arista a la GET
    auto agregarArista = [&](int x1, int y1, int x2, int y2) {
        // Ignorar aristas horizontales (no contribuyen al relleno)
        if (y1 == y2) {
            return;
        }
        
        // Asegurar que (x1,y1) sea el punto inferior
        if (y1 > y2) {
            std::swap(y1, y2);
            std::swap(x1, x2);
        }

        // Calcular pendiente inversa (dx/dy)
        const double dy = double(y2 - y1);
        const double dx = double(x2 - x1);
        const double pendienteInversa = dx / dy;

        // Aplicar convención de límites: [ceil(ymin), ceil(ymax))
        int y_minimo = int(std::ceil(y1));
        int y_maximo = int(std::ceil(y2));
        
        if (y_minimo == y_maximo) {
            return;  // Arista demasiado pequeña después del redondeo
        }

        // Calcular X en la primera línea de barrido
        double x_inicial = double(x1) + pendienteInversa * (double(y_minimo) - double(y1));

        // Determinar bucket en la GET
        int indiceBucket = y_minimo - yMinOut;
        if (indiceBucket >= 0 && indiceBucket < (int)GET.size()) {
            GET[indiceBucket].push_back(Edge{y_maximo, x_inicial, pendienteInversa});
        }
    };

    // Procesar todas las aristas del polígono (incluyendo cierre)
    for (int i = 0; i < numVertices; ++i) {
        auto [x1, y1] = vtx[i];
        auto [x2, y2] = vtx[(i + 1) % numVertices];  // Cierra el polígono
        agregarArista(x1, y1, x2, y2);
    }
    
    return GET;
}

/**
 * Rellena una línea horizontal entre dos puntos X
 */
static inline void fillSpan(Image &img, int y, int x0, int x1, const RGB &col)
{
    // Asegurar que x0 <= x1
    if (x0 > x1) {
        std::swap(x0, x1);
    }
    
    // Rellenar todos los píxeles entre x0 y x1 (inclusive)
    for (int x = x0; x <= x1; ++x) {
        img.putUser(x, y, col);
    }
}

/**
 * Algoritmo principal de relleno por scanline
 * 
 * ALGORITMO:
 * 1. Construir GET (Global Edge Table) con todas las aristas
 * 2. Para cada línea de barrido Y:
 *    a) Mover aristas de GET[Y] a AET (Active Edge Table)
 *    b) Eliminar de AET las aristas que terminan en Y
 *    c) Ordenar AET por coordenada X
 *    d) Rellenar entre pares de intersecciones (regla par-impar)
 *    e) Actualizar coordenadas X de las aristas activas
 */
static void scanlineFillPolygon(Image &img, const std::vector<std::pair<int, int>> &vtx, const RGB &fillColor)
{
    // Validar polígono
    if (vtx.size() < 3) {
        return;  // Polígono inválido
    }

    // Construir la Global Edge Table
    int yMinimo, yMaximo;
    auto GET = buildGET(vtx, yMinimo, yMaximo);
    if (GET.empty()) {
        return;  // No hay aristas válidas
    }

    // Inicializar Active Edge Table
    std::vector<Edge> AET;

    // BUCLE PRINCIPAL: procesar cada línea de barrido
    for (int y = yMinimo; y < yMaximo; ++y) {
        
        // PASO 1: Mover aristas que empiezan en Y desde GET hacia AET
        int indiceBucket = y - yMinimo;
        if (indiceBucket >= 0 && indiceBucket < (int)GET.size()) {
            for (const auto &arista : GET[indiceBucket]) {
                AET.push_back(arista);
            }
        }

        // PASO 2: Eliminar de AET las aristas que terminan en Y
        AET.erase(
            std::remove_if(AET.begin(), AET.end(),
                          [y](const Edge &arista) {
                              return arista.yMax <= y;
                          }),
            AET.end()
        );

        // Si no hay aristas activas, continuar con la siguiente línea
        if (AET.empty()) {
            continue;
        }

        // PASO 3: Ordenar AET por coordenada X (y yMax como criterio secundario)
        std::sort(AET.begin(), AET.end(),
                  [](const Edge &a, const Edge &b) {
                      if (a.x != b.x) {
                          return a.x < b.x;  // Ordenar por X principalmente
                      }
                      return a.yMax < b.yMax;  // Criterio de desempate
                  });

        // PASO 4: Rellenar entre pares de intersecciones (regla par-impar)
        for (size_t i = 0; i + 1 < AET.size(); i += 2) {
            double xIzquierda = AET[i].x;
            double xDerecha = AET[i + 1].x;
            
            // Convertir a coordenadas enteras de píxel
            int x0 = (int)std::ceil(xIzquierda);
            int x1 = (int)std::floor(xDerecha - 1e-9);  // Pequeño epsilon para evitar errores de redondeo
            
            // Rellenar si hay píxeles válidos entre las intersecciones
            if (x0 <= x1) {
                fillSpan(img, y, x0, x1, fillColor);
            }
        }

        // PASO 5: Actualizar coordenadas X para la siguiente línea de barrido
        for (auto &arista : AET) {
            arista.x += arista.invSlope;
        }
    }
}

// ============================================================================
// SECCIÓN 3: FUNCIÓN PRINCIPAL
// ============================================================================

/**
 * Función principal del programa
 * 
 * Parámetros de línea de comandos:
 * 1. salida.ppm - Nombre del archivo de imagen de salida
 * 2. W - Ancho de la imagen en píxeles
 * 3. H - Alto de la imagen en píxeles  
 * 4. vertices.txt - Archivo con los vértices del polígono
 * 5-7. [R G B] - Color de relleno (opcional, por defecto negro)
 */
int main(int argc, char **argv)
{
    // Verificar argumentos mínimos
    if (argc < 5) {
        std::cerr << "=== TP6 - Relleno de Polígonos por Scanline ===\n"
                  << "Uso: ./tp6 salida.ppm W H vertices.txt [R G B]\n"
                  << "\n"
                  << "Ejemplos:\n"
                  << "  ./tp6 poligono.ppm 120 80 vertices_practico6.txt\n"
                  << "  ./tp6 poligono_rojo.ppm 120 80 vertices_practico6.txt 255 0 0\n";
        return 1;
    }

    // Parsear argumentos de línea de comandos
    const std::string archivoSalida = argv[1];
    const int ancho = std::atoi(argv[2]);
    const int alto = std::atoi(argv[3]);
    const std::string archivoVertices = argv[4];

    // Validar dimensiones de la imagen
    if (ancho <= 0 || alto <= 0) {
        std::cerr << "Error: El ancho y alto deben ser mayores que 0.\n"
                  << "Valores recibidos: W=" << ancho << ", H=" << alto << "\n";
        return 1;
    }

    // Configurar colores
    RGB colorRelleno = rgbClamp(0, 0, 0);  // Negro por defecto
    if (argc >= 8) {
        // Color personalizado desde argumentos
        colorRelleno = rgbClamp(std::atoi(argv[5]), std::atoi(argv[6]), std::atoi(argv[7]));
    }
    const RGB colorFondo = rgbClamp(255, 255, 255);  // Fondo blanco

    // Cargar vértices del polígono
    std::vector<std::pair<int, int>> poligono;
    if (!loadVerticesFile(archivoVertices, poligono)) {
        std::cerr << "Error: No se pudieron cargar los vértices desde '" << archivoVertices << "'\n"
                  << "\n"
                  << "Causas posibles:\n"
                  << "- El archivo no existe o no se puede leer\n"
                  << "- El archivo tiene menos de 3 vértices válidos\n"
                  << "- Formato incorrecto (esperado: 'id x y' o 'x y' por línea)\n";
        return 1;
    }

    // Crear imagen y realizar relleno
    Image imagen(ancho, alto, colorFondo);
    scanlineFillPolygon(imagen, poligono, colorRelleno);
    
    // Guardar imagen resultante
    imagen.writePPM(archivoSalida);

    // Mostrar resumen de la operación
    std::cout << "\n=== TP6 - Relleno por Scanline Completado ===\n"
              << "Vértices procesados: " << poligono.size() << " (desde " << archivoVertices << ")\n"
              << "Dimensiones imagen: " << ancho << " x " << alto << " píxeles\n"
              << "Color de relleno: RGB(" << int(colorRelleno.r) << ", " 
                                        << int(colorRelleno.g) << ", " 
                                        << int(colorRelleno.b) << ")\n"
              << "Archivo generado: " << archivoSalida << "\n"
              << "\n"
              << "El polígono ha sido rellenado exitosamente usando el algoritmo Scanline.\n";
    
    return 0;
}
