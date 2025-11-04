// Ray Tracer Implementation
// Computación Gráfica - Práctico 2

#include <iostream>
#include <chrono>
#include <memory>

// GLM para matemáticas 3D
#include <glm/glm.hpp>

// Ray Tracer headers
#include "raytracer/Ray.h"
#include "raytracer/Camera.h"
#include "raytracer/Material.h"
#include "raytracer/HitRecord.h"
#include "raytracer/Object.h"
#include "raytracer/Sphere.h"
#include "raytracer/AABB.h"
#include "raytracer/Light.h"
#include "raytracer/Scene.h"
#include "raytracer/RayTracer.h"
#include "raytracer/ImageWriter.h"
#include "scenes/Scenes.h"

// Configuración de la imagen
static const int IMAGE_WIDTH = 800;
static const int IMAGE_HEIGHT = 600;

// Las escenas ahora viven en scenes/Scenes.{h,cpp}

void runRayTracer(int sceneChoice = 0) {
    std::cout << "=== Ray Tracer - Práctico 2 ==="<< std::endl;
    std::cout << "Resolución: " << IMAGE_WIDTH << "x" << IMAGE_HEIGHT << std::endl;
    
    // Crear la escena según la elección
    Scene scene = scenes::createByChoice(sceneChoice);
    const auto& info = scenes::infoForChoice(sceneChoice);
    const std::string sceneName = info.key;
    std::cout << "Escena: " << info.name << std::endl;
    
    // Configurar la cámara
    Camera camera(
        glm::vec3(0.0f, 1.0f, 2.0f),    // Posición
        glm::vec3(0.0f, 0.0f, -5.0f),   // Look at
        glm::vec3(0.0f, 1.0f, 0.0f),    // Up
        60.0f,                           // FOV
        float(IMAGE_WIDTH) / float(IMAGE_HEIGHT),  // Aspect ratio
        1.0f                             // Near plane
    );
    
    // Crear el ray tracer
    RayTracer raytracer(IMAGE_WIDTH, IMAGE_HEIGHT, 3);  // Max 3 reflexiones
    
    // Medir tiempo de renderizado
    std::cout << "\nRenderizando escena..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    // Renderizar la escena
    std::vector<glm::vec3> image = raytracer.render(scene, camera);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Tiempo de renderizado: " << diff.count() << " segundos" << std::endl;
    
    // Guardar las imágenes
    std::cout << "\nGuardando imágenes..." << std::endl;
    std::string ppmFile = sceneName + ".ppm";
    std::string bmpFile = sceneName + ".bmp";
    ImageWriter::writePPM(ppmFile, image, IMAGE_WIDTH, IMAGE_HEIGHT);
    ImageWriter::writeBMP(bmpFile, image, IMAGE_WIDTH, IMAGE_HEIGHT);
    
    std::cout << "\n✓ Ray tracing completado exitosamente!" << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        int sceneChoice = 0;
        
        if (argc > 1) {
            // Si se pasa un argumento, usarlo como número de escena
            sceneChoice = std::atoi(argv[1]);
            runRayTracer(sceneChoice);
        } else {
            // Menú interactivo
            std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
            std::cout << "║       RAY TRACER - PRÁCTICO 2         ║" << std::endl;
            std::cout << "║      Computación Gráfica 2025         ║" << std::endl;
            std::cout << "╚════════════════════════════════════════╝\n" << std::endl;
            
            std::cout << "Seleccione una escena para renderizar:\n" << std::endl;
            std::cout << "  1. Escena principal (esfera + cubo + múltiples luces)" << std::endl;
            std::cout << "  2. Test de sombras" << std::endl;
            std::cout << "  3. Test de materiales (variación de shininess)" << std::endl;
            std::cout << "  4. Test de atenuación por distancia" << std::endl;
            std::cout << "  5. Test de múltiples luces de colores" << std::endl;
            std::cout << "  0. Renderizar todas las escenas\n" << std::endl;
            
            std::cout << "Ingrese su elección (0-5): ";
            std::cin >> sceneChoice;
            
            if (sceneChoice == 0) {
                // Renderizar todas las escenas
                std::cout << "\nRenderizando todas las escenas...\n" << std::endl;
                for (int i = 1; i <= 5; ++i) {
                    std::cout << "\n--- Escena " << i << " de 5 ---" << std::endl;
                    runRayTracer(i);
                }
            } else {
                runRayTracer(sceneChoice);
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
