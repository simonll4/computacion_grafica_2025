#include "Altimeter.h"
#include <cmath>
#include <iostream>

namespace hud {

// Colores profesionales de aviación - Verde HUD
const glm::vec4 Altimeter::HUD_GREEN = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f);  // Verde HUD con leve cyan
const glm::vec4 Altimeter::HUD_WHITE = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f);  // También verde
const glm::vec4 Altimeter::HUD_YELLOW = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f); // También verde
const glm::vec4 Altimeter::BACKGROUND_COLOR = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); // Transparente

// Layout del altímetro (constantes en píxeles de pantalla)
static const float PX_PER_100FT = 24.0f;     // píxeles entre marcas de 100 ft
static const float TICK_MAJOR = 16.0f;        // largo tick mayor  
static const float LABEL_PAD = 6.0f;          // separación tick→texto
static const float BOX_W = 120.0f;            // ancho ventana de lectura
static const float BOX_H = 44.0f;             // alto ventana de lectura
static const float CHEVRON_W = 10.0f;         // ancho chevron
static const float CHEVRON_H = 12.0f;         // alto chevron

Altimeter::Altimeter() : position_(0.0f), size_(100.0f, 400.0f) {
}

void Altimeter::render(gfx::Renderer2D& renderer, const flight::FlightData& flightData) {
    float altitude = flightData.altitude;
    
    drawBackground(renderer);
    drawAltitudeTape(renderer, altitude);
    drawCurrentAltitudeBox(renderer, altitude);
}

void Altimeter::drawBackground(gfx::Renderer2D& renderer) {
    // SIN FONDO - solo elementos verdes
}

void Altimeter::drawAltitudeNumber(gfx::Renderer2D& renderer, int altitude, const glm::vec2& position) {
    std::string altStr = std::to_string(altitude);
    
    // Dígitos TABULARES monoespaciados (sin kerning)
    const float digitWidth = 8.0f;
    const float digitHeight = 12.0f;
    const float digitSpacing = 10.0f;  // Advance fijo (tabular)
    const float segmentThickness = 1.5f;
    
    // Centrar el string completo horizontalmente
    float totalWidth = altStr.length() * digitSpacing - (digitSpacing - digitWidth);
    float startX = position.x - totalWidth * 0.5f;
    
    // Dibujar cada dígito con spacing fijo (tabular)
    for (size_t i = 0; i < altStr.length(); ++i) {
        char digit = altStr[i];
        
        // Posición del dígito con pixel snapping
        float digitX = floor(startX + i * digitSpacing) + 0.5f;
        float digitY = floor(position.y - digitHeight * 0.5f) + 0.5f;
        glm::vec2 digitPos = glm::vec2(digitX, digitY);
        
        // Renderizar 7-segmentos
        drawDigit7Segment(renderer, digit, digitPos, digitWidth, digitHeight, segmentThickness);
    }
}

void Altimeter::drawDigit7Segment(gfx::Renderer2D& renderer, char digit, const glm::vec2& pos, float w, float h, float thickness) {
    // Posiciones de los 7 segmentos
    float halfH = h * 0.5f;
    
    // Definir qué segmentos están activos para cada dígito
    bool segments[7] = {false}; // a, b, c, d, e, f, g
    
    switch (digit) {
        case '0': segments[0] = segments[1] = segments[2] = segments[3] = segments[4] = segments[5] = true; break;
        case '1': segments[1] = segments[2] = true; break;
        case '2': segments[0] = segments[1] = segments[6] = segments[4] = segments[3] = true; break;
        case '3': segments[0] = segments[1] = segments[6] = segments[2] = segments[3] = true; break;
        case '4': segments[5] = segments[6] = segments[1] = segments[2] = true; break;
        case '5': segments[0] = segments[5] = segments[6] = segments[2] = segments[3] = true; break;
        case '6': segments[0] = segments[5] = segments[4] = segments[3] = segments[2] = segments[6] = true; break;
        case '7': segments[0] = segments[1] = segments[2] = true; break;
        case '8': segments[0] = segments[1] = segments[2] = segments[3] = segments[4] = segments[5] = segments[6] = true; break;
        case '9': segments[0] = segments[1] = segments[2] = segments[3] = segments[5] = segments[6] = true; break;
    }
    
    // Dibujar segmentos activos
    if (segments[0]) // a - top
        renderer.drawRect(pos + glm::vec2(thickness, 0), glm::vec2(w - 2*thickness, thickness), HUD_WHITE, true);
    if (segments[1]) // b - top right
        renderer.drawRect(pos + glm::vec2(w - thickness, thickness), glm::vec2(thickness, halfH - thickness), HUD_WHITE, true);
    if (segments[2]) // c - bottom right
        renderer.drawRect(pos + glm::vec2(w - thickness, halfH), glm::vec2(thickness, halfH - thickness), HUD_WHITE, true);
    if (segments[3]) // d - bottom
        renderer.drawRect(pos + glm::vec2(thickness, h - thickness), glm::vec2(w - 2*thickness, thickness), HUD_WHITE, true);
    if (segments[4]) // e - bottom left
        renderer.drawRect(pos + glm::vec2(0, halfH), glm::vec2(thickness, halfH - thickness), HUD_WHITE, true);
    if (segments[5]) // f - top left
        renderer.drawRect(pos + glm::vec2(0, thickness), glm::vec2(thickness, halfH - thickness), HUD_WHITE, true);
    if (segments[6]) // g - middle
        renderer.drawRect(pos + glm::vec2(thickness, halfH - thickness*0.5f), glm::vec2(w - 2*thickness, thickness), HUD_WHITE, true);
}

void Altimeter::drawAltitudeTape(gfx::Renderer2D& renderer, float altitude) {
    // Layout: centro Y y columna de ticks (rail)
    float centerY = position_.y + size_.y * 0.5f;
    float railX = position_.x + size_.x - 15.0f;  // Columna de ticks a la derecha
    
    // Computar estado de la cinta (base/frac estable)
    float base = floor(altitude / 100.0f) * 100.0f;
    float frac = (altitude - base) / 100.0f;  // [0,1)
    float yOff = frac * PX_PER_100FT;         // positivo: cinta baja al aumentar altitud
    
    // Dibujar rango de items [-N, +N] CADA 100 PIES
    const int N = 12;
    
    for (int i = -N; i <= N; ++i) {
        int v = (int)base + i * 100;  // Valor en ft (cada 100)
        float y = centerY + yOff - i * PX_PER_100FT;  // INVERTIDO: valores mayores ARRIBA (y menor)
        
        // Culling: fuera de viewport
        float cy = position_.y + size_.y * 0.5f;
        if (y < position_.y - 30.0f || y > position_.y + size_.y + 30.0f) continue;
        
        // Ocultar números que están dentro del box (área del cuadro digital)
        // BOX_H = 44px, así que ocultamos en rango ±22px desde el centro
        if (y > cy - BOX_H*0.5f && y < cy + BOX_H*0.5f) continue;
        
        // Pixel snapping: alinear a media-píxel para nitidez
        y = floor(y) + 0.5f;
        
        // Todos los ticks son iguales (cada 100 ft)
        float tickLen = TICK_MAJOR;
        
        // Dibujar tick horizontal (rectángulo de 1px alto)
        renderer.drawRect(glm::vec2(railX - tickLen, y - 0.5f), 
                         glm::vec2(tickLen, 1.0f), 
                         HUD_GREEN, true);
        
        // Etiqueta CADA 100 PIES (todos los ticks tienen número)
        if (v >= 0) {
            // Posición fija para columna alineada (right-aligned)
            float labelX = railX - tickLen - LABEL_PAD;
            
            // Dibujar número
            glm::vec2 numPos = glm::vec2(labelX - 30.0f, y);
            drawAltitudeNumber(renderer, v, numPos);
        }
    }
}

void Altimeter::drawCurrentAltitudeBox(gfx::Renderer2D& renderer, float altitude) {
    float cy = position_.y + size_.y * 0.5f;
    
    // Caja centrada en la cinta
    float boxX = position_.x + (size_.x - BOX_W) * 0.5f;
    float boxY = cy - BOX_H * 0.5f;
    
    // Borde del box (solo borde verde, sin fondo)
    renderer.drawRect(glm::vec2(boxX, boxY), 
                     glm::vec2(BOX_W, BOX_H), 
                     HUD_GREEN, false);
    
    // Chevron (uña) a la izquierda apuntando al centro
    float chevX = boxX - CHEVRON_W;
    float chevYTop = cy - CHEVRON_H * 0.5f;
    float chevYBot = cy + CHEVRON_H * 0.5f;
    
    // Triángulo isósceles apuntando a la derecha
    renderer.drawLine(glm::vec2(chevX, chevYTop), 
                     glm::vec2(boxX, cy), HUD_GREEN, 2.0f);
    renderer.drawLine(glm::vec2(boxX, cy), 
                     glm::vec2(chevX, chevYBot), HUD_GREEN, 2.0f);
    renderer.drawLine(glm::vec2(chevX, chevYTop), 
                     glm::vec2(chevX, chevYBot), HUD_GREEN, 2.0f);
    
    // Mostrar valor actual COMPLETO (simplificado, sin rodillo por ahora)
    int a = (int)round(altitude);
    if (a < 0) a = 0;
    
    // Dibujar el número completo centrado en el box
    glm::vec2 numPos = glm::vec2(boxX + BOX_W * 0.5f, cy);
    drawAltitudeNumber(renderer, a, numPos);
}

void Altimeter::drawAltitudeBug(gfx::Renderer2D& renderer, float targetAltitude) {
    float centerY = position_.y + size_.y * 0.5f;
    float bugY = centerY - altitudeToPixel(targetAltitude);
    
    // Solo dibujar si está visible
    if (bugY < position_.y || bugY > position_.y + size_.y) return;
    
    // Triángulo indicador
    float bugX = position_.x;
    glm::vec2 bugTip = glm::vec2(bugX - 8.0f, bugY);
    glm::vec2 bugTop = glm::vec2(bugX, bugY - 6.0f);
    glm::vec2 bugBottom = glm::vec2(bugX, bugY + 6.0f);
    
    // Dibujar triángulo
    renderer.drawLine(bugTip, bugTop, HUD_YELLOW, 2.0f);
    renderer.drawLine(bugTip, bugBottom, HUD_YELLOW, 2.0f);
    renderer.drawLine(bugTop, bugBottom, HUD_YELLOW, 2.0f);
}

float Altimeter::altitudeToPixel(float altitude) {
    // Convertir altitud a posición en píxeles
    return altitude * 0.5f;
}

float Altimeter::pixelToAltitude(float pixel) {
    return pixel / 0.5f;
}

} // namespace hud
