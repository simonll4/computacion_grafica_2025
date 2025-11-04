#include "WaypointRenderer.h"
#include <cmath>
#include <vector>
#include <iostream>

namespace gfx
{
    WaypointRenderer::~WaypointRenderer()
    {
        if (vao_)
            glDeleteVertexArrays(1, &vao_);
        if (vbo_)
            glDeleteBuffers(1, &vbo_);
        if (ebo_)
            glDeleteBuffers(1, &ebo_);
    }

    void WaypointRenderer::init()
    {
        // Compilar shaders
        shader_.loadFromSource(
            // Vertex Shader
            R"(
                #version 330 core
                layout(location = 0) in vec3 aPos;
                layout(location = 1) in vec3 aNormal;

                uniform mat4 model;
                uniform mat4 view;
                uniform mat4 projection;

                out vec3 FragPos;
                out vec3 Normal;

                void main()
                {
                    FragPos = vec3(model * vec4(aPos, 1.0));
                    Normal = mat3(transpose(inverse(model))) * aNormal;
                    gl_Position = projection * view * vec4(FragPos, 1.0);
                }
            )",
            // Fragment Shader
            R"(
                #version 330 core
                out vec4 FragColor;

                in vec3 FragPos;
                in vec3 Normal;

                uniform vec4 waypointColor;
                uniform vec3 viewPos;
                uniform bool isActive;

                void main()
                {
                    // Iluminación simple
                    vec3 norm = normalize(Normal);
                    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
                    float diff = max(dot(norm, lightDir), 0.0);
                    
                    // Ambiente
                    float ambient = 0.4;
                    
                    // Efecto de brillo si es activo
                    float glow = isActive ? 0.4 : 0.0;
                    
                    vec3 result = waypointColor.rgb * (ambient + diff + glow);
                    FragColor = vec4(result, waypointColor.a);
                }
            )");

        createCylinderGeometry();
    }

    void WaypointRenderer::createCylinderGeometry()
    {
        const int segments = 16;
        const float radius = 3.0f;
        const float height = 30.0f;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // Generar vértices del cilindro
        for (int i = 0; i <= segments; ++i)
        {
            float angle = (float)i / (float)segments * 2.0f * M_PI;
            float x = radius * cosf(angle);
            float z = radius * sinf(angle);

            // Vértice inferior
            vertices.push_back(x);
            vertices.push_back(0.0f);
            vertices.push_back(z);
            // Normal
            vertices.push_back(x / radius);
            vertices.push_back(0.0f);
            vertices.push_back(z / radius);

            // Vértice superior
            vertices.push_back(x);
            vertices.push_back(height);
            vertices.push_back(z);
            // Normal
            vertices.push_back(x / radius);
            vertices.push_back(0.0f);
            vertices.push_back(z / radius);
        }

        // Generar índices
        for (int i = 0; i < segments; ++i)
        {
            int base = i * 2;
            // Triángulo 1
            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            // Triángulo 2
            indices.push_back(base + 1);
            indices.push_back(base + 3);
            indices.push_back(base + 2);
        }

        indexCount_ = indices.size();

        // Crear buffers
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);

        glBindVertexArray(vao_);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Posición
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // Normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void WaypointRenderer::drawWaypoint(const glm::mat4 &view, const glm::mat4 &proj,
                                        const glm::vec3 &position, const glm::vec4 &color,
                                        bool isActive)
    {
        shader_.use();

        // Crear matriz de modelo
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);

        shader_.setMat4("model", model);
        shader_.setMat4("view", view);
        shader_.setMat4("projection", proj);
        shader_.setVec4("waypointColor", color);
        shader_.setBool("isActive", isActive);

        // Obtener viewPos desde la matriz view (inversa)
        glm::mat4 viewInv = glm::inverse(view);
        glm::vec3 viewPos = glm::vec3(viewInv[3]);
        shader_.setVec3("viewPos", viewPos);

        glBindVertexArray(vao_);
        glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

} // namespace gfx
