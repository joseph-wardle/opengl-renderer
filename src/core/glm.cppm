module;

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 0 // keep OpenGL default
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

export module core.glm;

// Export GLM vector/matrix types so their member functions/operators are visible
export using glm::vec2;
export using glm::vec3;
export using glm::vec4;
export using glm::mat3;
export using glm::mat4;

// Re-export free operators at module scope (not inside core namespace)
export using glm::operator+;
export using glm::operator-;
export using glm::operator*;
export using glm::operator/;

// Re-export functions you use at module scope (optional; you can also keep these inside core)
export using glm::radians;
export using glm::value_ptr;
export using glm::translate;
export using glm::rotate;
export using glm::scale;
export using glm::lookAt;
export using glm::perspective;
export using glm::length;
export using glm::normalize;
export using glm::cross;
export using glm::two_pi;

export namespace core {

// Convenience aliases
using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

// You can also expose wrappers via core if preferred
inline Mat4 mul(const Mat4& a, const Mat4& b) {
    return a * b;
}

}
