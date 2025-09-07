// #pragma once
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>

// struct Transform {
//     glm::vec3 position{0.0f, 0.0f, 0.0f};
//     glm::vec3 rotationEuler{0.0f, 0.0f, 0.0f}; // (x,y,z) rad
//     glm::vec3 scale{1.0f, 1.0f, 1.0f};

//     glm::mat4 model() const {
//         using namespace glm;
//         mat4 M(1.0f);
//         M = translate(M, position);
//         M = rotate(M, rotationEuler.z, vec3(0,0,1));
//         M = rotate(M, rotationEuler.y, vec3(0,1,0));
//         M = rotate(M, rotationEuler.x, vec3(1,0,0));
//         M = glm::scale(M, scale);
//         return M;
//     }
// };
