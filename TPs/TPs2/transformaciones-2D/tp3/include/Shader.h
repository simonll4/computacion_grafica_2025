#pragma once
#include <string>
#include <unordered_map>
extern "C" {
  #include <glad/glad.h>
}

class Shader {
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void use() const;

    // Setters típicos
    void setBool (const char* name, bool value);
    void setInt  (const char* name, int value);
    void setFloat(const char* name, float value);

    // Matrices y vectores (puntero a float para interoperar con GLM)
    void setMat4 (const char* name, const float* m16);
    void setVec4 (const char* name, float x, float y, float z, float w);
    void setVec3 (const char* name, float x, float y, float z);
    void setVec2 (const char* name, float x, float y);

    GLuint id() const { return program_; }

private:
    GLuint program_ = 0;
    std::unordered_map<std::string, GLint> uniformCache_;

    static std::string readFile(const char* path);
    static void checkCompile(GLuint shader, const char* stageName);
    static void checkLink(GLuint program);

    GLint getUniformLocation(const char* name);
};
