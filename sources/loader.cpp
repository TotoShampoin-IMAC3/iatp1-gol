#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <stdexcept>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "loader.hpp"

GLuint loadShader(const std::filesystem::path& file, const GLuint& type) {
    std::ifstream shader_file(file);
    if (!shader_file) {
        throw std::runtime_error(std::format("{} not found", file.string()));
    }
    std::string shader_source((std::istreambuf_iterator<char>(shader_file)), (std::istreambuf_iterator<char>()));
    auto shader_source_c = shader_source.c_str();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_source_c, NULL);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        throw std::runtime_error(std::format("Shader failed:\n{}", info_log));
    }
    return shader;
}

GLuint loadShaderProgram(const std::filesystem::path& vertex_file, const std::filesystem::path& frament_file) {
    GLuint program = glCreateProgram();
    GLuint vertex = loadShader(vertex_file.c_str(), GL_VERTEX_SHADER);
    GLuint fragment = loadShader(frament_file.c_str(), GL_FRAGMENT_SHADER);
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    glLinkProgram(program);
    return program;
}

GLuint loadComputeProgram(const std::filesystem::path& compute_file) {
    GLuint program = glCreateProgram();
    GLuint compute = loadShader(compute_file.c_str(), GL_COMPUTE_SHADER);
    glAttachShader(program, compute);
    glDeleteShader(compute);
    glLinkProgram(program);
    return program;
}

std::vector<float> loadImage(const std::filesystem::path& file) {
    int w, h, d;
    stbi_set_flip_vertically_on_load(false);
    auto img = stbi_load(file.c_str(), &w, &h, &d, 0);
    if (!img) {
        const char* failureReason = stbi_failure_reason();
        throw std::runtime_error(failureReason);
    }
    std::vector<float> image(w * h);
    for (int i = 0; i < w * h; i++) {
        image[i] = img[i * d] > 0 ? 1.0f : 0.0f;
    }
    stbi_image_free(img);
    return image;
}

GLuint loadTexture(const std::filesystem::path& file) {
    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glGenTextures(1, &texture);
    int w, h, d;
    stbi_set_flip_vertically_on_load(false);
    auto img = stbi_load(file.c_str(), &w, &h, &d, 0);
    if (!img) {
        const char* failureReason = stbi_failure_reason();
        throw std::runtime_error(failureReason);
    }
    GLenum format;
    switch (d) {
    case 1: format = GL_RED; break;
    case 2: format = GL_RG; break;
    case 3: format = GL_RGB; break;
    case 4: format = GL_RGBA; break;
    default: break;
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    stbi_image_free(img);
    return texture;
}

GLuint createTexture(int width, int height, GLenum internalformat, GLenum format, GLenum type, const void* data) {
    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, type, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return texture;
}

void replaceTexture(
    GLuint texture, int width, int height, GLenum internalformat, GLenum format, GLenum type, const void* data
) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, type, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void reloadTexture(GLuint texture, const std::filesystem::path& file) {
    int w, h, d;
    stbi_set_flip_vertically_on_load(true);
    auto img = stbi_load(file.c_str(), &w, &h, &d, 0);
    if (!img) { // NOLINT
        const char* failureReason = stbi_failure_reason();
        throw std::runtime_error(failureReason);
    }
    GLenum format;
    switch (d) {
    case 1: format = GL_RED; break;
    case 2: format = GL_RG; break;
    case 3: format = GL_RGB; break;
    case 4: format = GL_RGBA; break;
    default: break;
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(img);
}

void getTexture(GLuint texture, GLenum format, GLenum type, void* data) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexImage(GL_TEXTURE_2D, 0, format, type, data);
}

GLuint createFramebuffer(GLuint texture, GLuint renderbuffer) {
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    if (renderbuffer) {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    }
    return framebuffer;
}
