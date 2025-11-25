export module render.camera;

import std;
import core.glm;
import platform.glfw;
import core.app;

export namespace render {

class Camera {
public:
    Camera(core::Vec3 position, float aspect)
        : position_(position), aspect_(aspect) {}

    void set_aspect(float aspect) { aspect_ = aspect; }

    void update(core::DeltaTime dt, const platform::InputState& input) {
        const float d = dt.seconds;
        const float move_speed = base_speed_ * (input.is_down(platform::Key::left_shift) ? speed_multiplier_ : 1.0f);
        const core::Vec3 forward = forward_vector();
        const core::Vec3 right   = normalize(cross(forward, world_up_));

        if (input.is_down(platform::Key::w)) position_ += forward * move_speed * d;
        if (input.is_down(platform::Key::s)) position_ -= forward * move_speed * d;
        if (input.is_down(platform::Key::a)) position_ -= right * move_speed * d;
        if (input.is_down(platform::Key::d)) position_ += right * move_speed * d;
        if (input.is_down(platform::Key::e)) position_.y += move_speed * d;
        if (input.is_down(platform::Key::q)) position_.y -= move_speed * d;

        const auto mouse = input.mouse_delta();
        yaw_   += static_cast<float>(mouse.x) * mouse_sensitivity_;
        pitch_ -= static_cast<float>(mouse.y) * mouse_sensitivity_; // invert Y for natural feel

        pitch_ = std::clamp(pitch_, -radians(89.0f), radians(89.0f));
    }

    [[nodiscard]] core::Mat4 view() const {
        const auto forward = forward_vector();
        return lookAt(position_, position_ + forward, world_up_);
    }

    [[nodiscard]] core::Mat4 projection() const {
        return perspective(radians(fov_degrees_), aspect_, near_, far_);
    }

    [[nodiscard]] const core::Vec3& position() const noexcept { return position_; }

private:
    core::Vec3 position_{0.0f, 0.0f, 5.0f};
    float      yaw_{radians(-90.0f)};
    float      pitch_{0.0f};
    float      aspect_{16.0f / 9.0f};
    float      fov_degrees_{60.0f};
    float      near_{0.1f};
    float      far_{100.0f};
    float      base_speed_{3.0f};
    float      speed_multiplier_{2.0f};
    float      mouse_sensitivity_{0.0025f};
    core::Vec3 world_up_{0.0f, 1.0f, 0.0f};

    [[nodiscard]] core::Vec3 forward_vector() const {
        const float cy = std::cos(yaw_);
        const float sy = std::sin(yaw_);
        const float cp = std::cos(pitch_);
        const float sp = std::sin(pitch_);
        core::Vec3 f{cy * cp, sp, sy * cp};
        return normalize(f);
    }
};

} // namespace render
