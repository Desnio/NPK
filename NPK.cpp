#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include <iostream>
#include <filesystem>

#include "NPKLoader.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLFWwindow* initialise();

int main()
{
    GLFWwindow* window = initialise();

    std::vector<std::string> files;
    std::vector<const char*> items;
    std::vector<std::string> paks;
    std::vector<const char*> pakI;

    for(auto& file : std::filesystem::directory_iterator("."))
    {
        if(std::filesystem::is_directory(file.path()))
        {
            files.push_back(file.path().relative_path().string());
        }

    }

    for(auto& file : std::filesystem::recursive_directory_iterator(".", std::filesystem::directory_options::skip_permission_denied))
    {
        if(file.path().filename() == "Pak_dir.npk")
        {
            paks.push_back(file.path());
        }
    }

    for(auto& file : files)
    {
        items.push_back(file.c_str());
    }

    for(auto& pak : paks)
    {
        pakI.push_back(pak.c_str());
    }

    int current = 0;
    int current_dir = 0;

    std::unique_ptr<NPK> npk;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Create archives");

        ImGui::Combo("Folders", &current, items.data(), items.size());
        ImGui::Combo("Pak_dir", &current_dir, pakI.data(), pakI.size());

        if(ImGui::Button("Load Pak_dir"))
        {
            npk = std::make_unique<NPK>(paks[current_dir]);
        }

        if(ImGui::Button("Archive folder"))
        {
        //    NPKPacker.PackFolder(items[current], 12, 300);
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLFWwindow* initialise()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWmonitor* mon = glfwGetPrimaryMonitor();
    const GLFWvidmode* mod = glfwGetVideoMode(mon);
    GLFWwindow* window = glfwCreateWindow(mod->width, mod->height, "NPK", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    return window;
}
