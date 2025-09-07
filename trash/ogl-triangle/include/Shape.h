#pragma once

extern "C"
{
#include <glad/glad.h>
}

#include <vector>
#include "Shader.h"

enum class ShapeType {
    TRIANGLE,
    RECTANGLE,
    CIRCLE,
    LINE
};

class Shape
{
public:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    ShapeType type;
    
    // Constructor
    Shape(ShapeType shapeType);
    
    // Setup shape data
    void setupTriangle();
    void setupRectangle();
    void setupCircle(float radius = 0.5f, int segments = 32);
    void setupLine(float x1, float y1, float x2, float y2);
    
    // Setup OpenGL buffers
    void setupBuffers();
    
    // Draw the shape
    void draw(Shader& shader);
    
    // Set color uniform
    void setColor(Shader& shader, float r, float g, float b, float a = 1.0f);
    
    // Destructor
    ~Shape();

private:
    void generateBuffers();
};
