
// CGyAV - Guía 2 - Práctico 1
// Elipse con orientación arbitraria (Midpoint + Rotación + Unión anti-huecos)
// Parámetros (UN SOLO ORDEN):
//   ./tp1_ellipse_rotated cx cy rx ry angleDeg W H salida.ppm [R G B]
//
// Sistema de coordenadas:
// - Usuario: (0,0) abajo-izquierda, y crece hacia ARRIBA.
// - PPM (P6): filas de arriba hacia abajo -> hacemos flip vertical al volcar.
//
// Notas de implementación:
// - Midpoint elipse en 1er cuadrante (enteros, 2 regiones).
// - Rotamos TODOS los puntos muestreados (y sus 4 simetrías).
// - Redondeo estable (floor(v+0.5)) para evitar half-to-even.
// - Conectamos cada muestra rotada con la anterior (por simetría) con Bresenham
//   para eliminar huecos/granos. No agregamos “caps” de extremos.
// - Casos degenerados: rx=0 o ry=0 -> segmentos rotados.

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

//-------------------------------- Utilidades básicas --------------------------------

struct Color { uint8_t r=0,g=0,b=0; };

struct Image {
    int W, H;
    vector<uint8_t> pix; // RGB

    Image(int W, int H, Color bg = {255,255,255}) : W(W), H(H), pix(W*H*3) {
        clear(bg);
    }
    void clear(Color c) {
        for (int i=0;i<W*H;i++) {
            pix[3*i+0]=c.r; pix[3*i+1]=c.g; pix[3*i+2]=c.b;
        }
    }
    // (x,y) en coords usuario (0,0 abajo-izq)
    inline void putUser(int x, int y, Color c) {
        if (x<0 || x>=W || y<0 || y>=H) return; // clipping básico
        int row = (H - 1 - y); // flip vertical PPM
        int idx = (row*W + x)*3;
        pix[idx+0]=c.r; pix[idx+1]=c.g; pix[idx+2]=c.b;
    }
    void writePPM(const string& path) {
        ofstream f(path, ios::binary);
        if(!f){ cerr<<"No puedo abrir "<<path<<" para escribir.\n"; return; }
        f << "P6\n" << W << " " << H << "\n255\n";
        f.write((char*)pix.data(), pix.size());
    }
};

// Redondeo “nearest, half-up” (evita half-to-even).
static inline int iround(double v) { return (int)std::floor(v + 0.5); }

// Bresenham entero (línea de 1 px).
static void drawLine(Image& img, int x0,int y0,int x1,int y1, Color c) {
    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = dx + dy;
    while (true) {
        img.putUser(x0,y0,c);
        if (x0==x1 && y0==y1) break;
        int e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

struct Pt { int x, y; };

//------------------------------ Rotación precomputada ------------------------------

struct Rotator {
    double cos_a, sin_a;
    int cx, cy; // centro en coords usuario
    Rotator(double angleDeg, int cx, int cy) : cx(cx), cy(cy) {
        double a = angleDeg * M_PI / 180.0;
        cos_a = std::cos(a);
        sin_a = std::sin(a);
    }
    // Rota (ux,uy) alrededor de (cx,cy) y redondea estable
    inline Pt apply(int ux, int uy) const {
        double tx = double(ux - cx);
        double ty = double(uy - cy);
        double rx = tx * cos_a - ty * sin_a;
        double ry = tx * sin_a + ty * cos_a;
        return Pt{ iround(rx + cx), iround(ry + cy) };
    }
};

//------------------------- Midpoint elipse (primer cuadrante) -----------------------

static vector<Pt> ellipseMidpointFirstQuadrant(int rx, int ry) {
    vector<Pt> quad;
    if (rx < 0 || ry < 0) return quad;
    if (rx==0 && ry==0) { quad.push_back({0,0}); return quad; }

    long long rx2 = 1LL*rx*rx;
    long long ry2 = 1LL*ry*ry;

    long long x = 0;
    long long y = ry;

    long long dx = 2*ry2*x;
    long long dy = 2*rx2*y;

    long long d1 = ry2 - rx2*ry + rx2/4; // Región 1
    while (dx < dy) {
        quad.push_back({int(x), int(y)});
        if (d1 < 0) {
            x++; dx += 2*ry2;
            d1 += dx + ry2;
        } else {
            x++; y--; dx += 2*ry2; dy -= 2*rx2;
            d1 += dx - dy + ry2;
        }
    }

    // Región 2
    long long d2 = ry2*(x + 0.5)*(x + 0.5) + rx2*(y - 1)*(y - 1) - rx2*ry2;
    while (y >= 0) {
        quad.push_back({int(x), int(y)});
        if (d2 > 0) {
            y--; dy -= 2*rx2;
            d2 += rx2 - dy;
        } else {
            x++; y--; dx += 2*ry2; dy -= 2*rx2;
            d2 += dx - dy + rx2;
        }
    }
    return quad;
}

//-------------------- Dibujo de elipse rotada  -----------------------

static void drawRotatedEllipseContour(Image& img,
                                      int cx, int cy, int rx, int ry,
                                      double angleDeg, Color col)
{
    // Degenerados -> segmentos/punto rotados
    if (rx==0 && ry==0) { img.putUser(cx,cy,col); return; }
    if (rx==0 && ry>0) { // segmento vertical
        Rotator R(angleDeg, cx, cy);
        Pt a = R.apply(cx, cy+ry);
        Pt b = R.apply(cx, cy-ry);
        drawLine(img, a.x, a.y, b.x, b.y, col);
        return;
    }
    if (ry==0 && rx>0) { // segmento horizontal
        Rotator R(angleDeg, cx, cy);
        Pt a = R.apply(cx+rx, cy);
        Pt b = R.apply(cx-rx, cy);
        drawLine(img, a.x, a.y, b.x, b.y, col);
        return;
    }

    auto quad = ellipseMidpointFirstQuadrant(rx, ry);
    if (quad.empty()) return;

    Rotator R(angleDeg, cx, cy);

    // Para cada simetría guardamos el punto rotado previo:
    // 0:(+,+)  1:(-,+)  2:(+,-)  3:(-,-)
    array<optional<Pt>, 4> prev = { nullopt, nullopt, nullopt, nullopt };

    auto emit = [&](int sx, int sy, int slot, int x, int y) {
        // Generar simetría trasladada
        int ux = cx + sx * x;
        int uy = cy + sy * y;
        // Rotar y redondear estable
        Pt r = R.apply(ux, uy);

        if (prev[slot]) {
            Pt p = prev[slot].value();
            if (!(p.x==r.x && p.y==r.y)) {
                drawLine(img, p.x, p.y, r.x, r.y, col);
            } else {
                img.putUser(r.x, r.y, col);
            }
        } else {
            img.putUser(r.x, r.y, col);
        }
        prev[slot] = r;
    };

    // Recorremos muestras del 1er cuadrante conectando por simetría
    for (size_t i=0; i<quad.size(); ++i) {
        int x = quad[i].x;
        int y = quad[i].y;
        emit(+1,+1,0,x,y);
        emit(-1,+1,1,x,y);
        emit(+1,-1,2,x,y);
        emit(-1,-1,3,x,y);
    }

    // Importante: NO agregamos “caps” (extremos rotados adicionales).
    // Esto evita píxeles sueltos fuera del contorno.
}

//-------------------------------------- main --------------------------------------

int main(int argc, char** argv) {
    if (argc < 9) {
        cerr << "Uso:\n  " << argv[0]
             << " cx cy rx ry angleDeg W H salida.ppm [R G B]\n";
        return 1;
    }

    int cx, cy, rx, ry, W, H;
    double angleDeg;
    string outPath;
    Color stroke{0,0,0};
    const Color bg{255,255,255};

    auto clamp255 = [](int v){ return std::max(0, std::min(255, v)); };

    try {
        cx = stoi(argv[1]);
        cy = stoi(argv[2]);
        rx = stoi(argv[3]);
        ry = stoi(argv[4]);
        angleDeg = stod(argv[5]);
        W = stoi(argv[6]);
        H = stoi(argv[7]);
        outPath = argv[8];
        if (argc >= 12) {
            stroke.r = (uint8_t)clamp255(stoi(argv[9]));
            stroke.g = (uint8_t)clamp255(stoi(argv[10]));
            stroke.b = (uint8_t)clamp255(stoi(argv[11]));
        }
    } catch (const std::exception& e) {
        cerr << "Error parseando argumentos: " << e.what() << "\n";
        cerr << "Uso: " << argv[0]
             << " cx cy rx ry angleDeg W H salida.ppm [R G B]\n";
        return 1;
    }

    if (W<=0 || H<=0) { cerr<<"W,H inválidos.\n"; return 1; }
    if (rx<0 || ry<0) { cerr<<"rx,ry deben ser >= 0.\n"; return 1; }

    Image img(W, H, bg);
    drawRotatedEllipseContour(img, cx, cy, rx, ry, angleDeg, stroke);
    img.writePPM(outPath);

    // Resumen
    cout << "---- Elipse Rotada ----\n"
         << "Centro: ("<<cx<<","<<cy<<"), rx="<<rx<<", ry="<<ry<<"\n"
         << "Ángulo: "<<angleDeg<<"°\n"
         << "Lienzo: "<<W<<"x"<<H<<"\n"
         << "Trazo: ("<<(int)stroke.r<<","<<(int)stroke.g<<","<<(int)stroke.b<<")\n"
         << "Salida: "<<outPath<<"\n";

    return 0;
}