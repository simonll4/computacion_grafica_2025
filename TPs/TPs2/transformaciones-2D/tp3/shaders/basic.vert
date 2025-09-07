#version 460 core
layout(location = 0) in vec2 aPos;     // sirve para triángulo y círculo
uniform mat4 uProj, uView, uModel;
void main() {
    gl_Position = uProj * uView * uModel * vec4(aPos, 0.0, 1.0);
}


// #version 460 core
// layout(location=0) in vec3 aPos;

// uniform mat4 uModel;
// uniform mat4 uView;
// uniform mat4 uProj;

// void main() {
//     gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
// }
