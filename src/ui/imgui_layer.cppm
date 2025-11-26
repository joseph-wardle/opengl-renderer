module;

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

export module ui.imgui_layer;

import platform.glfw;

export namespace ui {

class ImGuiLayer {
public:
    explicit ImGuiLayer(platform::Window& window) {
        IMGUI_CHECKVERSION();
        context_ = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        // Docking exists only on the docking branch; guard so tagged releases still compile.
#ifdef ImGuiConfigFlags_DockingEnable
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif

        ImGui::StyleColorsDark();

        GLFWwindow* native = window.native_handle();
        // Let ImGui install callbacks so it can consume input; it will chain ours.
        ImGui_ImplGlfw_InitForOpenGL(native, true);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;

    ImGuiLayer(ImGuiLayer&&) = delete;
    ImGuiLayer& operator=(ImGuiLayer&&) = delete;

    ~ImGuiLayer() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(context_);
    }

    void begin_frame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void end_frame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

private:
    ImGuiContext* context_{nullptr};
};

} // namespace ui
