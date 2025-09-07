// SimpleMesh.h
#pragma once
extern "C" {
  #include <glad/glad.h>
}
#include <vector>
#include <glm/glm.hpp>

class SimpleMesh {
public:
  // Construye con posiciones 2D (z=0) o 3D. 'mode' puede ser GL_TRIANGLES, GL_LINE_LOOP, etc.
  SimpleMesh(const std::vector<float>& positions, GLenum mode = GL_TRIANGLES);
  SimpleMesh(const std::vector<glm::vec2>& positions2D, GLenum mode = GL_TRIANGLES);
  SimpleMesh(const std::vector<glm::vec3>& positions3D, GLenum mode = GL_TRIANGLES);

  ~SimpleMesh();

  void bind() const;
  void draw() const;

  // Helpers de construcción
  static SimpleMesh MakeCircle2D(float radius, int segments, GLenum mode = GL_LINE_LOOP);
  static SimpleMesh MakeTriangle2D(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);

private:
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLsizei m_count = 0;
  GLenum  m_mode  = GL_TRIANGLES;

  void initFromFloats(const std::vector<float>& positions, GLenum mode);
};


// #pragma once
// #include <vector>
// #include <cstdint>
// extern "C" {
//   #include <glad/glad.h>
// }

// class SimpleMesh {
// public:
//     // Posiciones puras: 3 floats por vértice
//     explicit SimpleMesh(const std::vector<float>& positions,
//                         bool useIndices=false,
//                         const std::vector<uint32_t>& indices = {});
//     ~SimpleMesh();

//     // no copiable
//     SimpleMesh(const SimpleMesh&) = delete;
//     SimpleMesh& operator=(const SimpleMesh&) = delete;

//     // movible
//     SimpleMesh(SimpleMesh&& other) noexcept;
//     SimpleMesh& operator=(SimpleMesh&& other) noexcept;

//     void bind() const;
//     void draw() const;

// private:
//     GLuint vao_=0, vbo_=0, ebo_=0;
//     GLsizei vertexCount_=0;
//     GLsizei indexCount_=0;
//     bool indexed_=false;

//     void destroy();
// };
