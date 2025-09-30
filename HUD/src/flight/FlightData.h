#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace flight {

struct FlightData {
    // Attitude (actitud)
    float pitch = 0.0f;      // Cabeceo en grados (-90 a +90)
    float roll = 0.0f;       // Alabeo en grados (-180 a +180)
    float yaw = 0.0f;        // Guiñada en grados (0 a 360)
    
    // Navigation
    float heading = 0.0f;    // Rumbo magnético en grados (0 a 360)
    float airspeed = 0.0f;   // Velocidad del aire en nudos
    float altitude = 1000.0f;   // Altitud en pies (inicia a 1000 ft)
    float verticalSpeed = 0.0f; // Velocidad vertical en pies/min
    
    // Position
    glm::vec3 position = glm::vec3(0.0f, 304.8f, 0.0f);  // y = 1000 ft en metros
    glm::vec3 velocity = glm::vec3(0.0f);
    
    // Camera-derived data
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
    
    // Update flight data from camera orientation
    void updateFromCamera(const glm::vec3& front, const glm::vec3& up, const glm::vec3& pos, float deltaTime);
    
    // Simulate flight physics
    void simulatePhysics(float deltaTime);
    
    // Helper functions
    float normalizeAngle(float angle);
    float degreesToRadians(float degrees) { return degrees * M_PI / 180.0f; }
    float radiansToDegrees(float radians) { return radians * 180.0f / M_PI; }
};

} // namespace flight
