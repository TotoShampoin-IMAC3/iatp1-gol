#include <cstdio>
#include <format>
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include "loader.hpp"
#include "rng.hpp"

constexpr int WINDOW_WIDTH = 720;
constexpr int WINDOW_HEIGHT = 640;

constexpr float FRAMERATE = 30.0f;
constexpr float FRAME_TIME = 1.0f / FRAMERATE;

constexpr int BUFFER_WIDTH = 320;
constexpr int BUFFER_HEIGHT = 240;

constexpr int FRAMEBUFFER_WIDTH = 640;
constexpr int FRAMEBUFFER_HEIGHT = 480;

std::vector<float> getRandomImage(int width, int height, float proba = .95) {
    static RandomNumberGenerator rng;

    auto image = std::vector<float>(width * height);
    for (auto& pixel : image) {
        pixel = rng() > proba ? 1.0f : 0.0f;
    }

    return image;
}

std::string openFile() {
    FILE* pipe = popen("kdialog --getopenfilename . '*.png *.jpg *.jpeg *.bmp *.tga'", "r");
    if (!pipe) {
        return "";
    }
    char buffer[512];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 512, pipe) != NULL) {
            result += buffer;
        }
    }
    pclose(pipe);
    return result.substr(0, result.length() - 1);
};

const char* ruleValue(int rule_val) {
    if (rule_val == 0) {
        return "Die";
    } else if (rule_val == 1) {
        return "Birth";
    } else if (rule_val == 2) {
        return "Survive";
    }
    return "Unknown";
}

int main(int argc, const char* argv[]) {
    glfwSetErrorCallback([](int error, const char* description) { fprintf(stderr, "Error: %s\n", description); });
    if (!glfwInit()) {
        return -1;
    }
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();

    GLuint framebuffer_texture =
        createTexture(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    GLuint framebuffer = createFramebuffer(framebuffer_texture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto image = getRandomImage(BUFFER_WIDTH, BUFFER_HEIGHT);
    GLuint buffer1 = createTexture(BUFFER_WIDTH, BUFFER_HEIGHT, GL_R32F, GL_RED, GL_FLOAT, image.data());
    GLuint buffer2 = createTexture(BUFFER_WIDTH, BUFFER_HEIGHT, GL_R32F, GL_RED, GL_FLOAT);
    glBindImageTexture(0, buffer1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glBindImageTexture(1, buffer2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

    GLuint compute = loadComputeProgram("resources/gol.comp");
    GLuint display = loadShaderProgram("resources/gol.vert", "resources/gol.frag");

    GLint u_texture = glGetUniformLocation(display, "u_texture");

    GLint u_resolution = glGetUniformLocation(compute, "u_resolution");

    GLint u_rules[9] = {glGetUniformLocation(compute, "u_rules[0]"), glGetUniformLocation(compute, "u_rules[1]"),
                        glGetUniformLocation(compute, "u_rules[2]"), glGetUniformLocation(compute, "u_rules[3]"),
                        glGetUniformLocation(compute, "u_rules[4]"), glGetUniformLocation(compute, "u_rules[5]"),
                        glGetUniformLocation(compute, "u_rules[6]"), glGetUniformLocation(compute, "u_rules[7]")};

    int rules[9] = {0, 0, 2, 1, 0, 0, 0, 0};
    bool is_updated = false;
    auto update_rules = [&] {
        glUseProgram(compute);
        for (int i = 0; i < 9; i++) {
            glUniform1i(u_rules[i], rules[i]);
        }
        is_updated = true;
    };
    float gen_proba = .05;

    int framerate = FRAMERATE;
    bool is_paused = false;

    glUseProgram(display);
    glBindTexture(GL_TEXTURE_2D, buffer2);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(u_texture, 0);

    glUseProgram(compute);
    update_rules();
    glUniform2i(u_resolution, BUFFER_WIDTH, BUFFER_HEIGHT);

    std::string file_path;

    struct State {
        glm::vec2 res = glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT);
    } state;
    glfwSetWindowUserPointer(window, &state);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        State& state = *static_cast<State*>(glfwGetWindowUserPointer(window));
        state.res = glm::vec2(width, height);
    });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    double last_time = glfwGetTime();
    do {
        double current_time = glfwGetTime();
        double elapsed_time = current_time - last_time;
        glfwPollEvents();

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT);
        if (!is_paused && elapsed_time > 1.f / framerate) {
            glUseProgram(compute);
            glBindImageTexture(0, buffer1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
            glBindImageTexture(1, buffer2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
            glDispatchCompute(BUFFER_WIDTH, BUFFER_HEIGHT, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glCopyImageSubData(
                buffer2, GL_TEXTURE_2D, 0, 0, 0, 0, buffer1, GL_TEXTURE_2D, 0, 0, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT, 1
            );
            last_time = current_time;
        }
        glUseProgram(display);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, state.res.x, state.res.y);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(state.res.x, state.res.y), ImGuiCond_Always);
        ImGui::Begin(
            "Debug", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoTitleBar
        );

        for (int i = 0; i < 9; i++) {
            if (i != 0) {
                ImGui::SameLine();
            }
            if (ImGui::Button(std::format("{}: {} ##{}", i, ruleValue(rules[i]), i).c_str())) {
                rules[i] = (rules[i] + 1) % 3;
                is_updated = false;
            }
        }
        if (!is_updated) {
            ImGui::SameLine();
            ImGui::Text("*");
        }
        if (ImGui::Button("Apply")) {
            update_rules();
        }
        ImGui::SameLine();
        if (ImGui::Button("Regenerate")) {
            image = getRandomImage(BUFFER_WIDTH, BUFFER_HEIGHT, 1.f - gen_proba);
            replaceTexture(buffer1, BUFFER_WIDTH, BUFFER_HEIGHT, GL_R32F, GL_RED, GL_FLOAT, image.data());
            update_rules();
        }
        ImGui::SameLine();
        ImGui::InputInt("Framerate", &framerate);

        if (ImGui::Button(is_paused ? "Resume##pause" : "Pause##pause")) {
            is_paused = !is_paused;
        }

        if (ImGui::Button("Open file")) {
            file_path = openFile();
            if (!file_path.empty()) {
                auto image = loadImage(file_path);
                replaceTexture(buffer1, BUFFER_WIDTH, BUFFER_HEIGHT, GL_R32F, GL_RED, GL_FLOAT, image.data());
            }
        }
        if (!file_path.empty()) {
            ImGui::SameLine();
            ImGui::Text("...%s", file_path.substr(file_path.length() - 64, 64).c_str());
        }

        ImGui::SliderFloat("Generation probability", &gen_proba, 0.0f, 1.0f);

        ImGui::Image((void*)(intptr_t)framebuffer_texture, ImVec2(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT));
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    } while (!glfwWindowShouldClose(window));

    glDeleteProgram(compute);
    glDeleteTextures(1, &buffer1);
    glDeleteTextures(1, &buffer2);
    glDeleteProgram(display);
    glfwDestroyWindow(window);
    // This segfaults for some reason
    // glfwTerminate();
    return 0;
}
