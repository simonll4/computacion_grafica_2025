// SimpleMesh.cpp
#include "SimpleMesh.h"
#include <cassert>
#include <cmath>

void SimpleMesh::initFromFloats(const std::vector<float>& positions, GLenum mode) {
  m_mode  = mode;
  // positions es [x,y,(z)] por vértice. Aceptamos vec2 (stride=2) o vec3 (stride=3).
  assert(!positions.empty());
  const bool isVec2 = (positions.size() % 2 == 0);
  const bool isVec3 = (positions.size() % 3 == 0);

  int comps = 0;
  if (isVec3 && !isVec2) comps = 3;
  else if (isVec2 && !isVec3) comps = 2;
  else if (isVec3 && isVec2)  comps = 3;   // preferimos 3 si es múltiplo de ambos
  else                        comps = 3;   // fallback

  m_count = (GLsizei)(positions.size() / comps);

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, positions.size()*sizeof(float), positions.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, comps, GL_FLOAT, GL_FALSE, comps*sizeof(float), (void*)0);

  glBindVertexArray(0);
}

SimpleMesh::SimpleMesh(const std::vector<float>& positions, GLenum mode) {
  initFromFloats(positions, mode);
}
SimpleMesh::SimpleMesh(const std::vector<glm::vec2>& positions2D, GLenum mode) {
  std::vector<float> flat; flat.reserve(positions2D.size()*2);
  for (auto& p : positions2D) { flat.push_back(p.x); flat.push_back(p.y); }
  initFromFloats(flat, mode);
}
SimpleMesh::SimpleMesh(const std::vector<glm::vec3>& positions3D, GLenum mode) {
  std::vector<float> flat; flat.reserve(positions3D.size()*3);
  for (auto& p : positions3D) { flat.push_back(p.x); flat.push_back(p.y); flat.push_back(p.z); }
  initFromFloats(flat, mode);
}

SimpleMesh::~SimpleMesh() {
  if (m_vbo) glDeleteBuffers(1, &m_vbo);
  if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void SimpleMesh::bind() const {
  glBindVertexArray(m_vao);
}

void SimpleMesh::draw() const {
  glDrawArrays(m_mode, 0, m_count);
}

// ----------- Helpers -----------
SimpleMesh SimpleMesh::MakeCircle2D(float radius, int segments, GLenum mode) {
  std::vector<glm::vec2> pts;
  pts.reserve(segments);
  const float TWOPI = 6.28318530718f;
  for (int i = 0; i < segments; ++i) {
    const float a = (TWOPI * i) / segments;
    pts.emplace_back(radius * std::cos(a), radius * std::sin(a));
  }
  return SimpleMesh(pts, mode);
}

SimpleMesh SimpleMesh::MakeTriangle2D(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
  std::vector<glm::vec2> pts = { a, b, c };
  return SimpleMesh(pts, GL_TRIANGLES);
}


// #include "SimpleMesh.h"

// SimpleMesh::SimpleMesh(const std::vector<float>& positions,
//                        bool useIndices,
//                        const std::vector<uint32_t>& indices)
// : indexed_(useIndices)
// {
//     glGenVertexArrays(1, &vao_);
//     glGenBuffers(1, &vbo_);
//     glBindVertexArray(vao_);

//     glBindBuffer(GL_ARRAY_BUFFER, vbo_);
//     glBufferData(GL_ARRAY_BUFFER,
//                  positions.size()*sizeof(float),
//                  positions.data(),
//                  GL_STATIC_DRAW);

//     if (indexed_ && !indices.empty()) {
//         glGenBuffers(1, &ebo_);
//         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
//         glBufferData(GL_ELEMENT_ARRAY_BUFFER,
//                      indices.size()*sizeof(uint32_t),
//                      indices.data(),
//                      GL_STATIC_DRAW);
//         indexCount_ = static_cast<GLsizei>(indices.size());
//     } else {
//         vertexCount_ = static_cast<GLsizei>(positions.size()/3);
//     }

//     // layout(location=0) -> vec3 posición
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
//                           3*sizeof(float), (void*)0);

//     glBindVertexArray(0);
// }

// SimpleMesh::~SimpleMesh() { destroy(); }

// void SimpleMesh::destroy() {
//     if (ebo_) glDeleteBuffers(1, &ebo_), ebo_=0;
//     if (vbo_) glDeleteBuffers(1, &vbo_), vbo_=0;
//     if (vao_) glDeleteVertexArrays(1, &vao_), vao_=0;
// }

// SimpleMesh::SimpleMesh(SimpleMesh&& o) noexcept {
//     vao_=o.vao_; vbo_=o.vbo_; ebo_=o.ebo_;
//     vertexCount_=o.vertexCount_; indexCount_=o.indexCount_;
//     indexed_=o.indexed_;
//     o.vao_=o.vbo_=o.ebo_=0; o.vertexCount_=o.indexCount_=0; o.indexed_=false;
// }

// SimpleMesh& SimpleMesh::operator=(SimpleMesh&& o) noexcept {
//     if (this != &o) {
//         destroy();
//         vao_=o.vao_; vbo_=o.vbo_; ebo_=o.ebo_;
//         vertexCount_=o.vertexCount_; indexCount_=o.indexCount_;
//         indexed_=o.indexed_;
//         o.vao_=o.vbo_=o.ebo_=0; o.vertexCount_=o.indexCount_=0; o.indexed_=false;
//     }
//     return *this;
// }

// void SimpleMesh::bind() const { glBindVertexArray(vao_); }

// void SimpleMesh::draw() const {
//     if (indexed_) glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, (void*)0);
//     else          glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
//     glBindVertexArray(0);
// }
