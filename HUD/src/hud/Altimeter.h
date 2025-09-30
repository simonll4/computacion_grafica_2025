#pragma once
#include <glm/glm.hpp>
#include "../gfx/Renderer2D.h"
#include "../flight/FlightData.h"

namespace hud {

class Altimeter {
public:
    Altimeter();
    
    void setPosition(const glm::vec2& position) { position_ = position; }
    void setSize(const glm::vec2& size) { size_ = size; }
    
    void render(gfx::Renderer2D& renderer, const flight::FlightData& flightData);
    
private:
    glm::vec2 position_;
    glm::vec2 size_;
    
    // Para el indicador de tendencia
    float lastAltitude_;
    
    // Colores
    static const glm::vec4 HUD_GREEN;
    static const glm::vec4 HUD_WHITE;
    static const glm::vec4 HUD_YELLOW;
    static const glm::vec4 BACKGROUND_COLOR;
    
    // Configuración de escala
    static const float MIN_ALTITUDE;
    static const float MAX_ALTITUDE;
    static const float MAJOR_TICK_INTERVAL;
    static const float MINOR_TICK_INTERVAL;
    
    void drawBackground(gfx::Renderer2D& renderer);
    void drawAltitudeTape(gfx::Renderer2D& renderer, float altitude);
    void drawCurrentAltitudeBox(gfx::Renderer2D& renderer, float altitude);
    void drawAltitudeBug(gfx::Renderer2D& renderer, float targetAltitude);
    void drawAltitudeNumber(gfx::Renderer2D& renderer, int altitude, const glm::vec2& position);
    void drawDigit7Segment(gfx::Renderer2D& renderer, char digit, const glm::vec2& pos, float w, float h, float thickness);
    
    float altitudeToPixel(float altitude);
    float pixelToAltitude(float pixel);
};

} // namespace hud
