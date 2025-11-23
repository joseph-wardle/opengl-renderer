// src/math/math.glm.cppm
module;

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 0 // keep OpenGL default
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

export module math.glm;

export namespace math {
    
using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

using glm::radians;
using glm::value_ptr;

}
