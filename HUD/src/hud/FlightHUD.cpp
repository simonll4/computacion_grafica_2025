#include "FlightHUD.h"
#include <iostream>

namespace hud {

// Definición de colores
const glm::vec4 FlightHUD::HUD_GREEN = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
const glm::vec4 FlightHUD::HUD_WHITE = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
const glm::vec4 FlightHUD::HUD_YELLOW = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
const glm::vec4 FlightHUD::HUD_RED = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
const glm::vec4 FlightHUD::HUD_CYAN = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);

FlightHUD::FlightHUD() : screenWidth_(1280), screenHeight_(720) {
    renderer2D_ = std::make_unique<gfx::Renderer2D>();
}

void FlightHUD::init(int screenWidth, int screenHeight) {
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;
    
    renderer2D_->init(screenWidth, screenHeight);
    
    // Configurar layout del altímetro
    setupAltimeterLayout();
    
    std::cout << "Altimeter HUD initialized: " << screenWidth << "x" << screenHeight << std::endl;
}

void FlightHUD::setScreenSize(int width, int height) {
    screenWidth_ = width;
    screenHeight_ = height;
    renderer2D_->setScreenSize(width, height);
    
    // Reconfigurar layout para nueva resolución
    setupAltimeterLayout();
}

void FlightHUD::update(const flight::FlightData& flightData) {
    currentFlightData_ = flightData;
}

void FlightHUD::render() {
    // Configurar blending para transparencia
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Deshabilitar depth test para el HUD (siempre encima)
    glDisable(GL_DEPTH_TEST);
    
    renderer2D_->begin();
    
    // Renderizar altímetro
    altimeter_.render(*renderer2D_, currentFlightData_);
    
    renderer2D_->end();
    
    // Reactivar depth test
    glEnable(GL_DEPTH_TEST);
    
    glDisable(GL_BLEND);
}

void FlightHUD::setLayout(const std::string& layoutName) {
    // Solo un layout para el altímetro
    setupAltimeterLayout();
}

void FlightHUD::setupAltimeterLayout() {
    float centerY = screenHeight_ * 0.5f;
    
    // Altímetro profesional en el lado derecho (como en HUDs reales)
    float altimeterWidth = 120.0f;
    float altimeterHeight = 450.0f;
    float marginRight = 30.0f;
    
    float posX = screenWidth_ - altimeterWidth - marginRight;
    float posY = centerY - altimeterHeight * 0.5f;
    
    altimeter_.setPosition(glm::vec2(posX, posY));
    altimeter_.setSize(glm::vec2(altimeterWidth, altimeterHeight));
}

} // namespace hud
