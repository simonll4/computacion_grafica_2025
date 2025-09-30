#include "flight/FlightData.h"
#include <cmath>
#include <algorithm>

namespace flight {

void FlightData::updateFromCamera(const glm::vec3& front, const glm::vec3& up, const glm::vec3& pos, float deltaTime) {
    cameraFront = front;
    cameraUp = up;
    cameraRight = glm::normalize(glm::cross(front, up));
    
    // Calcular pitch desde la dirección de la cámara
    // pitch = arcsin(front.y) pero limitado
    float newPitch = radiansToDegrees(asin(glm::clamp(front.y, -1.0f, 1.0f)));
    
    // Calcular roll desde la orientación up de la cámara
    // Proyectar el vector up en el plano perpendicular a front
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 rightInWorld = glm::normalize(glm::cross(front, worldUp));
    glm::vec3 upInWorld = glm::cross(rightInWorld, front);
    
    // El roll es el ángulo entre el up de la cámara y el up calculado
    float dotProduct = glm::dot(up, upInWorld);
    float crossProduct = glm::dot(glm::cross(up, upInWorld), front);
    float newRoll = radiansToDegrees(atan2(crossProduct, dotProduct));
    
    // Calcular heading (yaw) desde la proyección horizontal de front
    glm::vec3 frontHorizontal = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
    float newHeading = radiansToDegrees(atan2(frontHorizontal.x, -frontHorizontal.z));
    newHeading = normalizeAngle(newHeading);
    
    // Suavizar los cambios para evitar jitter
    float smoothFactor = 1.0f - exp(-10.0f * deltaTime);
    pitch = pitch + (newPitch - pitch) * smoothFactor;
    roll = roll + (newRoll - roll) * smoothFactor;
    heading = heading + (newHeading - heading) * smoothFactor;
    yaw = heading; // Para simplicidad, yaw = heading
    
    // Actualizar posición
    glm::vec3 deltaPos = pos - position;
    velocity = deltaPos / deltaTime;
    position = pos;
    
    // Calcular velocidades
    float speed = glm::length(velocity);
    airspeed = speed * 1.94384f; // m/s a nudos (aproximado)
    
    // Velocidad vertical (componente Y de la velocidad)
    verticalSpeed = velocity.y * 196.85f; // m/s a pies/min (aproximado)
    
    // Altitud con piso de referencia fijo
    // Piso en Y = 1.8 metros (altura de ojos) = 0 pies de altitud
    const float GROUND_LEVEL_Y = 1.8f;  // metros (altura de ojos del piloto)
    float heightAboveGround = position.y - GROUND_LEVEL_Y;
    altitude = heightAboveGround * 3.28084f; // metros a pies
}

void FlightData::simulatePhysics(float deltaTime) {
    // Simulación básica de física de vuelo
    // Esto se puede expandir con modelos más complejos
    
    // Aplicar gravedad a la velocidad vertical si no hay sustentación
    if (airspeed < 50.0f) { // Velocidad de pérdida simplificada
        verticalSpeed -= 500.0f * deltaTime; // Caída libre
    }
    
    // Limitar valores extremos
    pitch = glm::clamp(pitch, -90.0f, 90.0f);
    roll = glm::clamp(roll, -180.0f, 180.0f);
    airspeed = glm::max(0.0f, airspeed);
    verticalSpeed = glm::clamp(verticalSpeed, -6000.0f, 6000.0f); // Límites realistas
}

float FlightData::normalizeAngle(float angle) {
    while (angle < 0.0f) angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;
    return angle;
}

} // namespace flight
