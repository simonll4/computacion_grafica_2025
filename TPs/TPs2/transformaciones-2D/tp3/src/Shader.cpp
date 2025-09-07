#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

std::string Shader::readFile(const char* path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "[Shader] No se pudo abrir: " << path << "\n";
        return {};
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void Shader::checkCompile(GLuint shader, const char* stageName) {
    GLint ok=0; glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len=0; glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len);
        glGetShaderInfoLog(shader, len, nullptr, log.data());
        std::cerr << "[Shader] Error de compilación (" << stageName << "):\n"
                  << log.data() << "\n";
    }
}

void Shader::checkLink(GLuint program) {
    GLint ok=0; glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len=0; glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len);
        glGetProgramInfoLog(program, len, nullptr, log.data());
        std::cerr << "[Shader] Error de link:\n" << log.data() << "\n";
    }
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    const std::string vsrc = readFile(vertexPath);
    const std::string fsrc = readFile(fragmentPath);
    if (vsrc.empty() || fsrc.empty()) {
        std::cerr << "[Shader] Fuentes vacías. Abortando.\n";
        return;
    }

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    const char* vptr = vsrc.c_str();
    glShaderSource(vs, 1, &vptr, nullptr);
    glCompileShader(vs);
    checkCompile(vs, "vertex");

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fptr = fsrc.c_str();
    glShaderSource(fs, 1, &fptr, nullptr);
    glCompileShader(fs);
    checkCompile(fs, "fragment");

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);
    checkLink(program_);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader() {
    if (program_) glDeleteProgram(program_);
}

void Shader::use() const {
    glUseProgram(program_);
}

GLint Shader::getUniformLocation(const char* name) {
    auto it = uniformCache_.find(name);
    if (it != uniformCache_.end()) return it->second;
    GLint loc = glGetUniformLocation(program_, name);
    uniformCache_[name] = loc;
    return loc;
}

void Shader::setBool(const char* name, bool v)   { glUniform1i(getUniformLocation(name), (int)v); }
void Shader::setInt (const char* name, int v)    { glUniform1i(getUniformLocation(name), v); }
void Shader::setFloat(const char* name, float v) { glUniform1f(getUniformLocation(name), v); }

void Shader::setMat4(const char* name, const float* m16) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, m16);
}
void Shader::setVec4(const char* name, float x, float y, float z, float w) {
    glUniform4f(getUniformLocation(name), x,y,z,w);
}
void Shader::setVec3(const char* name, float x, float y, float z) {
    glUniform3f(getUniformLocation(name), x,y,z);
}
void Shader::setVec2(const char* name, float x, float y) {
    glUniform2f(getUniformLocation(name), x,y);
}
