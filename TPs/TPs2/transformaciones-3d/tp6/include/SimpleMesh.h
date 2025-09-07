#pragma once
#include <vector>
#include <cstdint>
extern "C" {
  #include <glad/glad.h>
}

class SimpleMesh {
public:
    // Posiciones puras: 3 floats por vértice
    explicit SimpleMesh(const std::vector<float>& positions,
                        bool useIndices=false,
                        const std::vector<uint32_t>& indices = {});
    ~SimpleMesh();

    // no copiable
    SimpleMesh(const SimpleMesh&) = delete;
    SimpleMesh& operator=(const SimpleMesh&) = delete;

    // movible
    SimpleMesh(SimpleMesh&& other) noexcept;
    SimpleMesh& operator=(SimpleMesh&& other) noexcept;

    void bind() const;
    void draw() const;

private:
    GLuint vao_=0, vbo_=0, ebo_=0;
    GLsizei vertexCount_=0;
    GLsizei indexCount_=0;
    bool indexed_=false;

    void destroy();
};
