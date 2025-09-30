#version 330 core

in vec3 FragColor;
out vec4 outColor;

void main() {
    outColor = vec4(FragColor, 1.0);
}
