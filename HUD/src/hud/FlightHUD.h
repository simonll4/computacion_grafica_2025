#pragma once
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "../gfx/Renderer2D.h"
#include "../flight/FlightData.h"

// Includes de instrumentos implementados
#include "Altimeter.h"

// TODO: Agregar includes de futuros instrumentos
// #include "AttitudeIndicator.h"
// #include "AirspeedIndicator.h"
// #include "HeadingIndicator.h"
// #include "VerticalSpeedIndicator.h"

namespace hud {
/**
 * @class FlightHUD
 * @brief Coordinador central de todos los instrumentos del HUD
 * 
 * Esta clase es el punto de entrada para el sistema de HUD. Gestiona:
 * - Creación y configuración de todos los instrumentos
 * - Actualización de datos de vuelo
 * - Renderizado coordinado de todos los instrumentos
 * - Layout y posicionamiento
 * - Manejo de cambios de resolución
 */
class FlightHUD {
public:
    FlightHUD();
    ~FlightHUD() = default;
    
    // ========================================================================
    // INICIALIZACIÓN Y CONFIGURACIÓN
    // ========================================================================
    
    void init(int screenWidth, int screenHeight);
    void setScreenSize(int width, int height);
    void setLayout(const std::string& layoutName);
    
    // ========================================================================
    // ACTUALIZACIÓN Y RENDERIZADO
    // ========================================================================
    
    void update(const flight::FlightData& flightData);
    void render();
    
private:
    // ========================================================================
    // SISTEMA DE RENDERIZADO
    // ========================================================================
    
    std::unique_ptr<gfx::Renderer2D> renderer2D_;
    
    // ========================================================================
    // INSTRUMENTOS DEL HUD
    // ========================================================================
    
    // Implementados:
    Altimeter altimeter_;
    
    // TODO: Agregar instrumentos futuros aquí
    // AttitudeIndicator attitudeIndicator_;
    // AirspeedIndicator airspeedIndicator_;
    // HeadingIndicator headingIndicator_;
    // VerticalSpeedIndicator verticalSpeedIndicator_;
    
    // ========================================================================
    // DATOS Y CONFIGURACIÓN
    // ========================================================================
    
    flight::FlightData currentFlightData_;
    int screenWidth_;
    int screenHeight_;
    
    // ========================================================================
    // CONFIGURACIÓN INTERNA
    // ========================================================================
    
    void setupInstrumentLayout();  // Configura layout de TODOS los instrumentos
};

} // namespace hud
