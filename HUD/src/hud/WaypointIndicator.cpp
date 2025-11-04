#include "WaypointIndicator.h"
#include "../gfx/TextRenderer.h"
#include <cmath>

namespace hud
{
    // ============================================================================
    // CONFIGURACIÓN VISUAL DEL INDICADOR
    // ============================================================================

    static const float CIRCLE_RADIUS = 60.0f;           // Radio del círculo de la brújula
    static const float ARROW_LENGTH = 40.0f;            // Longitud de la flecha direccional
    static const float ARROW_HEAD_SIZE = 12.0f;         // Tamaño de la punta de la flecha
    static const float DISTANCE_BOX_HEIGHT = 30.0f;     // Altura de la caja de distancia
    static const float HEADING_TICK_LENGTH = 8.0f;      // Longitud del tick de rumbo actual

    WaypointIndicator::WaypointIndicator() : Instrument()
    {
        // Configuración específica del indicador de waypoint
        size_ = glm::vec2(160.0f, 180.0f);
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f); // Verde HUD
    }

    // ============================================================================
    // FUNCIÓN PRINCIPAL DE RENDERIZADO
    // ============================================================================

    void WaypointIndicator::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        // Si no hay waypoint activo, mostrar mensaje
        if (!flightData.hasActiveWaypoint)
        {
            drawNoWaypointMessage(renderer);
            return;
        }

        drawBackground(renderer);
        drawDirectionArrow(renderer, flightData.waypointBearing, flightData.heading);
        drawDistanceInfo(renderer, flightData.waypointDistance);
    }

    // ============================================================================
    // RENDERIZADO DE FONDO Y CÍRCULO DE BRÚJULA
    // ============================================================================

    void WaypointIndicator::drawBackground(gfx::Renderer2D &renderer)
    {
        glm::vec2 center = position_ + size_ * 0.5f;
        center.y -= 15.0f; // Ajustar para dejar espacio para la distancia abajo

        // Dibujar círculo exterior de la brújula
        renderer.drawCircle(center, CIRCLE_RADIUS, color_, 32, false);

        // Dibujar cruz de referencia (N-S-E-W)
        float crossSize = CIRCLE_RADIUS * 0.9f;
        
        // Línea vertical (N-S)
        renderer.drawLine(
            glm::vec2(center.x, center.y - crossSize),
            glm::vec2(center.x, center.y + crossSize),
            glm::vec4(color_.r, color_.g, color_.b, 0.3f),
            1.0f);

        // Línea horizontal (E-W)
        renderer.drawLine(
            glm::vec2(center.x - crossSize, center.y),
            glm::vec2(center.x + crossSize, center.y),
            glm::vec4(color_.r, color_.g, color_.b, 0.3f),
            1.0f);

        // Dibujar "N" en la parte superior
        glm::vec2 northTextPos = glm::vec2(center.x, center.y - CIRCLE_RADIUS - 12.0f);
        gfx::TextRenderer::drawString(renderer, "N", northTextPos, glm::vec2(8.0f, 12.0f), color_, 10.0f);
    }

    // ============================================================================
    // RENDERIZADO DE FLECHA DIRECCIONAL
    // ============================================================================

    void WaypointIndicator::drawDirectionArrow(gfx::Renderer2D &renderer, float bearing, float heading)
    {
        glm::vec2 center = position_ + size_ * 0.5f;
        center.y -= 15.0f;

        // Calcular el ángulo relativo (diferencia entre el rumbo al waypoint y el rumbo actual)
        // Esto hará que la flecha apunte hacia donde el piloto debe girar
        float relativeAngle = bearing - heading;
        
        // Normalizar el ángulo al rango [-180, 180]
        while (relativeAngle > 180.0f) relativeAngle -= 360.0f;
        while (relativeAngle < -180.0f) relativeAngle += 360.0f;

        // Convertir a radianes (0° = arriba, sentido horario)
        float angleRad = glm::radians(relativeAngle - 90.0f);

        // Calcular punto final de la flecha
        glm::vec2 arrowEnd = center + glm::vec2(
            std::cos(angleRad) * ARROW_LENGTH,
            std::sin(angleRad) * ARROW_LENGTH
        );

        // Dibujar línea principal de la flecha
        renderer.drawLine(center, arrowEnd, color_, 3.0f);

        // Calcular puntos de la punta de la flecha
        float headAngle1 = angleRad + glm::radians(150.0f);
        float headAngle2 = angleRad - glm::radians(150.0f);

        glm::vec2 arrowHead1 = arrowEnd + glm::vec2(
            std::cos(headAngle1) * ARROW_HEAD_SIZE,
            std::sin(headAngle1) * ARROW_HEAD_SIZE
        );

        glm::vec2 arrowHead2 = arrowEnd + glm::vec2(
            std::cos(headAngle2) * ARROW_HEAD_SIZE,
            std::sin(headAngle2) * ARROW_HEAD_SIZE
        );

        // Dibujar punta de la flecha
        renderer.drawLine(arrowEnd, arrowHead1, color_, 3.0f);
        renderer.drawLine(arrowEnd, arrowHead2, color_, 3.0f);

        // Dibujar tick en la parte superior para indicar el rumbo actual del avión
        glm::vec2 tickStart = center + glm::vec2(0.0f, -CIRCLE_RADIUS);
        glm::vec2 tickEnd = center + glm::vec2(0.0f, -CIRCLE_RADIUS - HEADING_TICK_LENGTH);
        renderer.drawLine(tickStart, tickEnd, color_, 2.0f);

        // Mostrar el ángulo de desvío si es significativo
        if (std::abs(relativeAngle) > 5.0f)
        {
            int angleInt = static_cast<int>(std::round(relativeAngle));
            std::string angleStr = (angleInt > 0 ? "+" : "") + std::to_string(angleInt) + "°";
            
            glm::vec2 angleTextPos = center + glm::vec2(0.0f, 15.0f);
            gfx::TextRenderer::drawString(renderer, angleStr, angleTextPos, 
                glm::vec2(6.0f, 10.0f), color_, 8.0f);
        }
    }

    // ============================================================================
    // RENDERIZADO DE INFORMACIÓN DE DISTANCIA
    // ============================================================================

    void WaypointIndicator::drawDistanceInfo(gfx::Renderer2D &renderer, float distance)
    {
        glm::vec2 center = position_ + size_ * 0.5f;
        
        // Posición de la caja de distancia (abajo del círculo)
        float boxY = center.y + CIRCLE_RADIUS + 20.0f;
        float boxWidth = size_.x * 0.8f;
        float boxX = center.x - boxWidth * 0.5f;

        // Dibujar caja de fondo
        renderer.drawRect(
            glm::vec2(boxX, boxY),
            glm::vec2(boxWidth, DISTANCE_BOX_HEIGHT),
            color_,
            false);

        // Convertir distancia a kilómetros o metros según la magnitud
        std::string distText;
        if (distance >= 1000.0f)
        {
            float distKm = distance / 1000.0f;
            distText = std::to_string(static_cast<int>(std::round(distKm * 10.0f)) / 10.0f) + " km";
        }
        else
        {
            distText = std::to_string(static_cast<int>(std::round(distance))) + " m";
        }

        // Dibujar texto de distancia centrado
        glm::vec2 textPos = glm::vec2(center.x, boxY + DISTANCE_BOX_HEIGHT * 0.5f);
        gfx::TextRenderer::drawString(renderer, distText, textPos, 
            glm::vec2(8.0f, 12.0f), color_, 10.0f);
    }

    // ============================================================================
    // MENSAJE CUANDO NO HAY WAYPOINT ACTIVO
    // ============================================================================

    void WaypointIndicator::drawNoWaypointMessage(gfx::Renderer2D &renderer)
    {
        glm::vec2 center = position_ + size_ * 0.5f;

        // Dibujar círculo tenue
        renderer.drawCircle(center, CIRCLE_RADIUS * 0.7f, 
            glm::vec4(color_.r, color_.g, color_.b, 0.3f), 32, false);

        // Mensaje "NO WPT"
        glm::vec2 textPos = center;
        gfx::TextRenderer::drawString(renderer, "NO WPT", textPos, 
            glm::vec2(8.0f, 12.0f), 
            glm::vec4(color_.r, color_.g, color_.b, 0.5f), 
            10.0f);
    }

} // namespace hud
