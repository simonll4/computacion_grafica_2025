#include "Shape.h"
#include <cmath>

extern "C"
{
#include <GLFW/glfw3.h>
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Shape::Shape(ShapeType shapeType) : type(shapeType)
{
    generateBuffers();

    switch (type)
    {
    case ShapeType::TRIANGLE:
        setupTriangle();
        break;
    case ShapeType::RECTANGLE:
        setupRectangle();
        break;
    case ShapeType::CIRCLE:
        setupCircle();
        break;
    case ShapeType::LINE:
        setupLine(0.0f, 0.0f, 1.0f, 1.0f); // Default line
        break;
    }

    setupBuffers();
}

void Shape::setupTriangle()
{
    vertices = {
        -0.5f, -0.5f, 0.0f, // Bottom left
        0.5f, -0.5f, 0.0f,  // Bottom right
        0.0f, 0.5f, 0.0f    // Top center
    };

    indices = {0, 1, 2};
}

void Shape::setupRectangle()
{
    vertices = {
        0.5f, 0.5f, 0.0f,   // Top right
        0.5f, -0.5f, 0.0f,  // Bottom right
        -0.5f, -0.5f, 0.0f, // Bottom left
        -0.5f, 0.5f, 0.0f   // Top left
    };

    indices = {
        0, 1, 3, // First triangle
        1, 2, 3  // Second triangle
    };
}

void Shape::setupCircle(float radius, int segments)
{
    vertices.clear();
    indices.clear();

    // Center vertex
    vertices.push_back(0.0f); // x
    vertices.push_back(0.0f); // y
    vertices.push_back(0.0f); // z

    // Generate vertices around the circle
    for (int i = 0; i <= segments; ++i)
    {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        vertices.push_back(radius * cos(theta)); // x
        vertices.push_back(radius * sin(theta)); // y
        vertices.push_back(0.0f);                // z
    }

    // Generate indices for triangles
    for (int i = 0; i < segments; ++i)
    {
        indices.push_back(0);                            // Center
        indices.push_back(i + 1);                        // Current vertex
        indices.push_back(i + 2 > segments ? 1 : i + 2); // Next vertex (wrap around)
    }
}

void Shape::setupLine(float x1, float y1, float x2, float y2)
{
    vertices = {
        x1, y1, 0.0f, // Start point
        x2, y2, 0.0f  // End point
    };

    indices = {0, 1};
}

void Shape::generateBuffers()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
}

void Shape::setupBuffers()
{
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Shape::draw(Shader &shader)
{
    shader.use();
    glBindVertexArray(VAO);

    if (type == ShapeType::LINE)
    {
        glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}

void Shape::setColor(Shader &shader, float r, float g, float b, float a)
{
    shader.setVec4("color", r, g, b, a);
}

Shape::~Shape()
{
    // Only delete OpenGL objects if context is still valid
    if (glfwGetCurrentContext() != nullptr)
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
}
