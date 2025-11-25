module;

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 0 // keep OpenGL default
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

export module core.glm;

export namespace core {

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

using glm::radians;
using glm::value_ptr;
using glm::translate;
using glm::rotate;
using glm::scale;
using glm::lookAt;
using glm::perspective;
using glm::length;
using glm::normalize;
using glm::mat4;

inline Mat4 mul(const Mat4& a, const Mat4& b) {
    return a * b;
}

}
