#pragma once
#include <GL/gl.h>
#include <filesystem>

GLuint loadShader(const std::filesystem::path& file, const GLuint& type);
GLuint loadShaderProgram(const std::filesystem::path& vertex_file, const std::filesystem::path& frament_file);
GLuint loadComputeProgram(const std::filesystem::path& compute_file);
GLuint loadTexture(const std::filesystem::path& file);
GLuint createTexture(
    int width, int height, GLenum internalformat, GLenum format, GLenum type, const void* data = nullptr
);
void replaceTexture(
    GLuint texture, int width, int height, GLenum internalformat, GLenum format, GLenum type, const void* data = nullptr
);
void reloadTexture(GLuint texture, const std::filesystem::path& file);
void getTexture(GLuint texture, GLenum format, GLenum type, void* data);
GLuint createRenderbuffer(int width, int height);
GLuint createFramebuffer(GLuint texture, GLuint renderbuffer = 0);
