#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "../gfx/Renderer2D.h"
#include "../flight/FlightData.h"
#include "Altimeter.h"

namespace hud {

class FlightHUD {
public:
    FlightHUD();
    ~FlightHUD() = default;
    
    void init(int screenWidth, int screenHeight);
    void setScreenSize(int width, int height);
    
    void update(const flight::FlightData& flightData);
    void render();
    
    // Configuración de layout
    void setLayout(const std::string& layoutName);
    
private:
    std::unique_ptr<gfx::Renderer2D> renderer2D_;
    
    // Instrumento único
    Altimeter altimeter_;
    
    // Datos de vuelo actuales
    flight::FlightData currentFlightData_;
    
    // Configuración de pantalla
    int screenWidth_, screenHeight_;
    
    // Layout del altímetro
    void setupAltimeterLayout();
    
    // Colores del HUD
    static const glm::vec4 HUD_GREEN;
    static const glm::vec4 HUD_WHITE;
    static const glm::vec4 HUD_YELLOW;
    static const glm::vec4 HUD_RED;
    static const glm::vec4 HUD_CYAN;
};

} // namespace hud
