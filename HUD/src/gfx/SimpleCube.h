#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"

extern "C" {
#include <glad/glad.h>
}

namespace gfx {

class SimpleCube {
public:
    SimpleCube() = default;
    ~SimpleCube();

    void init();
    void draw(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& position = glm::vec3(0.0f));

private:
    GLuint vao_ = 0, vbo_ = 0;
    Shader shader_;
    
    void createGeometry();
};

} // namespace gfx
