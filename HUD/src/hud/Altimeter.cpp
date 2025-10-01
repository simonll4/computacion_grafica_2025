#include "Altimeter.h"
#include <cmath>

namespace hud
{
    // ============================================================================
    // CONFIGURACIÓN DE LA ESCALA DEL TAPE
    // ============================================================================

    // Espaciado entre marcas de altitud
    static const float ALTITUDE_STEP = 100.0f;  // Marcas cada 100 pies
    static const float PIXELS_PER_STEP = 30.0f; // Separación vertical entre marcas
    static const int VISIBLE_MARKS = 12;        // Cuántas marcas mostrar arriba/abajo del centro

    // ============================================================================
    // CONFIGURACIÓN VISUAL DEL TAPE
    // ============================================================================

    // Ticks (marcas laterales)
    static const float TICK_LENGTH = 16.0f;       // Longitud de las marcas horizontales
    static const float TICK_TO_NUMBER_GAP = 6.0f; // Espacio entre tick y número

    // Caja de lectura central
    static const float READOUT_BOX_WIDTH = 120.0f;
    static const float READOUT_BOX_HEIGHT = 44.0f;

    // Flecha indicadora (chevron)
    static const float CHEVRON_WIDTH = 10.0f;
    static const float CHEVRON_HEIGHT = 12.0f;

    Altimeter::Altimeter() : Instrument()
    {
        // Configuración específica del altímetro
        size_ = glm::vec2(100.0f, 400.0f);
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f); // Verde HUD
    }

    // ============================================================================
    // FUNCIÓN PRINCIPAL DE RENDERIZADO
    // ============================================================================

    void Altimeter::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        float altitude = flightData.altitude;

        drawBackground(renderer);
        drawAltitudeTape(renderer, altitude);
        drawCurrentAltitudeBox(renderer, altitude);
    }

    void Altimeter::drawBackground(gfx::Renderer2D &renderer)
    {
        // El altímetro no tiene fondo - solo dibujar elementos sobre el HUD transparente
    }

    // ============================================================================
    // RENDERIZADO DE NÚMEROS (7 SEGMENTOS)
    // ============================================================================

    void Altimeter::drawAltitudeNumber(gfx::Renderer2D &renderer, int altitude, const glm::vec2 &position)
    {
        std::string altStr = std::to_string(altitude);

        // Dígitos TABULARES monoespaciados (sin kerning)
        const float digitWidth = 8.0f;
        const float digitHeight = 12.0f;
        const float digitSpacing = 10.0f; // Advance fijo (tabular)
        const float segmentThickness = 1.5f;

        // Centrar el string completo horizontalmente
        float totalWidth = altStr.length() * digitSpacing - (digitSpacing - digitWidth);
        float startX = position.x - totalWidth * 0.5f;

        // Dibujar cada dígito con spacing fijo (tabular)
        for (size_t i = 0; i < altStr.length(); ++i)
        {
            char digit = altStr[i];

            // Posición del dígito con pixel snapping
            float digitX = floor(startX + i * digitSpacing) + 0.5f;
            float digitY = floor(position.y - digitHeight * 0.5f) + 0.5f;
            glm::vec2 digitPos = glm::vec2(digitX, digitY);

            // Renderizar 7-segmentos
            drawDigit7Segment(renderer, digit, digitPos, digitWidth, digitHeight, segmentThickness);
        }
    }

    void Altimeter::drawDigit7Segment(gfx::Renderer2D &renderer, char digit, const glm::vec2 &pos, float w, float h, float thickness)
    {
        // Posiciones de los 7 segmentos
        float halfH = h * 0.5f;

        // Definir qué segmentos están activos para cada dígito
        bool segments[7] = {false}; // a, b, c, d, e, f, g

        switch (digit)
        {
        case '0':
            segments[0] = segments[1] = segments[2] = segments[3] = segments[4] = segments[5] = true;
            break;
        case '1':
            segments[1] = segments[2] = true;
            break;
        case '2':
            segments[0] = segments[1] = segments[6] = segments[4] = segments[3] = true;
            break;
        case '3':
            segments[0] = segments[1] = segments[6] = segments[2] = segments[3] = true;
            break;
        case '4':
            segments[5] = segments[6] = segments[1] = segments[2] = true;
            break;
        case '5':
            segments[0] = segments[5] = segments[6] = segments[2] = segments[3] = true;
            break;
        case '6':
            segments[0] = segments[5] = segments[4] = segments[3] = segments[2] = segments[6] = true;
            break;
        case '7':
            segments[0] = segments[1] = segments[2] = true;
            break;
        case '8':
            segments[0] = segments[1] = segments[2] = segments[3] = segments[4] = segments[5] = segments[6] = true;
            break;
        case '9':
            segments[0] = segments[1] = segments[2] = segments[3] = segments[5] = segments[6] = true;
            break;
        }

        // Dibujar segmentos activos
        if (segments[0]) // a - top
            renderer.drawRect(pos + glm::vec2(thickness, 0), glm::vec2(w - 2 * thickness, thickness), color_, true);
        if (segments[1]) // b - top right
            renderer.drawRect(pos + glm::vec2(w - thickness, thickness), glm::vec2(thickness, halfH - thickness), color_, true);
        if (segments[2]) // c - bottom right
            renderer.drawRect(pos + glm::vec2(w - thickness, halfH), glm::vec2(thickness, halfH - thickness), color_, true);
        if (segments[3]) // d - bottom
            renderer.drawRect(pos + glm::vec2(thickness, h - thickness), glm::vec2(w - 2 * thickness, thickness), color_, true);
        if (segments[4]) // e - bottom left
            renderer.drawRect(pos + glm::vec2(0, halfH), glm::vec2(thickness, halfH - thickness), color_, true);
        if (segments[5]) // f - top left
            renderer.drawRect(pos + glm::vec2(0, thickness), glm::vec2(thickness, halfH - thickness), color_, true);
        if (segments[6]) // g - middle
            renderer.drawRect(pos + glm::vec2(thickness, halfH - thickness * 0.5f), glm::vec2(w - 2 * thickness, thickness), color_, true);
    }

    // ============================================================================
    // RENDERIZADO DEL TAPE DE ALTITUD (ESCALA MÓVIL)
    // ============================================================================

    void Altimeter::drawAltitudeTape(gfx::Renderer2D &renderer, float altitude)
    {
        // Calcular posiciones de referencia
        float centerY = position_.y + size_.y * 0.5f; // Centro vertical del instrumento
        float ticksX = position_.x + size_.x - 15.0f; // Columna donde van los ticks

        // Calcular el desplazamiento del tape basado en la altitud actual
        // Dividimos la altitud en parte entera (base) y fraccionaria
        float baseAltitude = floor(altitude / ALTITUDE_STEP) * ALTITUDE_STEP; // Ej: 234 ft → 200 ft
        float fraction = (altitude - baseAltitude) / ALTITUDE_STEP;           // Ej: 34/100 = 0.34
        float scrollOffset = fraction * PIXELS_PER_STEP;                      // Desplazamiento en píxeles

        // Dibujar marcas de altitud visibles
        for (int i = -VISIBLE_MARKS; i <= VISIBLE_MARKS; ++i)
        {
            // Calcular el valor de altitud para esta marca
            int markAltitude = (int)baseAltitude + i * (int)ALTITUDE_STEP; // Ej: 0, 100, 200, 300...

            // Calcular posición Y de esta marca en pantalla
            // Cuando subes: scrollOffset aumenta → tape sube (valores mayores aparecen desde arriba)
            float markY = centerY + scrollOffset - i * PIXELS_PER_STEP;

            // Saltar marcas fuera del área visible
            const float CULLING_MARGIN = 30.0f;
            if (markY < position_.y - CULLING_MARGIN || markY > position_.y + size_.y + CULLING_MARGIN)
                continue;

            // Verificar si está dentro del área de la caja de lectura
            bool insideReadoutBox = (markY > centerY - READOUT_BOX_HEIGHT * 0.5f &&
                                     markY < centerY + READOUT_BOX_HEIGHT * 0.5f);

            if (insideReadoutBox)
                continue; // Saltar marcas dentro de la caja

            // Dibujar el tick (línea horizontal)
            // NO usar floor() para evitar que múltiples marcas se redondeen al mismo píxel
            renderer.drawRect(
                glm::vec2(ticksX - TICK_LENGTH, markY - 0.5f),
                glm::vec2(TICK_LENGTH, 1.0f),
                color_,
                true);

            // Dibujar el número (solo si la altitud es positiva)
            if (markAltitude >= 0)
            {
                float numberX = ticksX - TICK_LENGTH - TICK_TO_NUMBER_GAP - 30.0f;
                glm::vec2 numberPos = glm::vec2(numberX, markY);
                drawAltitudeNumber(renderer, markAltitude, numberPos);
            }
        }
    }

    // ============================================================================
    // CAJA DE LECTURA DIGITAL (CENTRO)
    // ============================================================================

    void Altimeter::drawCurrentAltitudeBox(gfx::Renderer2D &renderer, float altitude)
    {
        float centerY = position_.y + size_.y * 0.5f;

        // Posicionar la caja en el centro del instrumento
        float boxX = position_.x + (size_.x - READOUT_BOX_WIDTH) * 0.5f;
        float boxY = centerY - READOUT_BOX_HEIGHT * 0.5f;

        // Dibujar marco de la caja (solo borde, sin relleno)
        renderer.drawRect(
            glm::vec2(boxX, boxY),
            glm::vec2(READOUT_BOX_WIDTH, READOUT_BOX_HEIGHT),
            color_,
            false // Sin relleno
        );

        // Dibujar flecha indicadora (chevron) a la izquierda de la caja
        float chevronX = boxX - CHEVRON_WIDTH;
        float chevronTopY = centerY - CHEVRON_HEIGHT * 0.5f;
        float chevronBotY = centerY + CHEVRON_HEIGHT * 0.5f;

        renderer.drawLine(
            glm::vec2(chevronX, chevronTopY),
            glm::vec2(boxX, centerY),
            color_, 2.0f);
        renderer.drawLine(
            glm::vec2(boxX, centerY),
            glm::vec2(chevronX, chevronBotY),
            color_, 2.0f);
        renderer.drawLine(
            glm::vec2(chevronX, chevronTopY),
            glm::vec2(chevronX, chevronBotY),
            color_, 2.0f);

        // Mostrar altitud actual redondeada (no negativa)
        int displayAltitude = (int)round(altitude);
        if (displayAltitude < 0)
            displayAltitude = 0;

        // Dibujar el número centrado dentro de la caja
        glm::vec2 numberPos = glm::vec2(boxX + READOUT_BOX_WIDTH * 0.5f, centerY);
        drawAltitudeNumber(renderer, displayAltitude, numberPos);
    }

} // namespace hud
