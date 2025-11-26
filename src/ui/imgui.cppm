module;

#include <imgui.h>

export module ui.imgui;
import std;

// Re-export the ImGui namespace for module consumers.
export namespace imgui {
    using IO            = ::ImGuiIO;
    using Context       = ::ImGuiContext;
    using DrawData      = ::ImDrawData;
    using WindowFlags   = ::ImGuiWindowFlags;
    using SliderFlags   = ::ImGuiSliderFlags;

    inline Context* CreateContext() { return ::ImGui::CreateContext(); }
    inline void     DestroyContext(Context* ctx = nullptr) { ::ImGui::DestroyContext(ctx); }

    inline IO&      GetIO() { return ::ImGui::GetIO(); }
    inline void     StyleColorsDark() { ::ImGui::StyleColorsDark(); }
    inline void     NewFrame() { ::ImGui::NewFrame(); }
    inline void     Render() { ::ImGui::Render(); }
    inline DrawData* GetDrawData() { return ::ImGui::GetDrawData(); }

    inline bool Begin(const char* name, bool* open = nullptr, WindowFlags flags = 0) {
        return ::ImGui::Begin(name, open, flags);
    }
    inline void End() { ::ImGui::End(); }

    template <typename... Args>
    inline void Text(Args&&... args) {
        ::ImGui::Text(std::forward<Args>(args)...);
    }

    inline void Separator() { ::ImGui::Separator(); }
    inline bool Checkbox(const char* label, bool* v) { return ::ImGui::Checkbox(label, v); }
    inline void BeginDisabled(bool disabled = true) { ::ImGui::BeginDisabled(disabled); }
    inline void EndDisabled() { ::ImGui::EndDisabled(); }
    inline bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* fmt = "%.3f", SliderFlags flags = 0) {
        return ::ImGui::SliderFloat(label, v, v_min, v_max, fmt, flags);
    }
}
