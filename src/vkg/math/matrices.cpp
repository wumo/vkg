#include "matrices.hpp"

namespace vkg {

auto viewMatrix(glm::vec3 eye, glm::vec3 center, glm::vec3 up) -> glm::mat4 {
    glm::vec3 const f(normalize(center - eye));
    glm::vec3 const s(normalize(cross(f, up)));
    glm::vec3 const u(cross(s, f));

    glm::mat4 Result(1);
    Result[0][0] = s.x;
    Result[1][0] = s.y;
    Result[2][0] = s.z;
    Result[0][1] = u.x;
    Result[1][1] = u.y;
    Result[2][1] = u.z;
    Result[0][2] = -f.x;
    Result[1][2] = -f.y;
    Result[2][2] = -f.z;
    Result[3][0] = -dot(s, eye);
    Result[3][1] = -dot(u, eye);
    Result[3][2] = dot(f, eye);
    return Result;
}

auto perspectiveMatrix(float fovy, float aspect, float zNear, float zFar) -> glm::mat4 {
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0);

    float const tanHalfFovy = glm::tan(fovy / 2.f);

    glm::mat4 result(0);
    result[0][0] = 1.f / (aspect * tanHalfFovy);
    result[1][1] = -1.f / (tanHalfFovy);
    result[2][2] = zFar / (zNear - zFar);
    result[2][3] = -1.f;
    result[3][2] = (zFar * zNear) / (zNear - zFar);
    return result;
}

auto orthoMatrix(float with, float height, float zNear, float zFar) -> glm::mat4 {
    glm::mat4 result(0);
    result[0][0] = 2.f / with;
    result[1][1] = -2.f / height;
    result[2][2] = 1.f / (zNear - zFar);
    result[3][2] = zNear / (zNear - zFar);
    result[3][3] = 1.f;
    return result;
}
auto orthoMatrix(float left, float right, float bottom, float top, float zNear, float zFar) -> glm::mat4 {
    glm::mat4 result(0);
    result[0][0] = 2.f / (right - left);
    result[1][1] = -2.f / (top - bottom);
    result[2][2] = 1.f / (zNear - zFar);
    result[3][0] = (left + right) / (left - right);
    result[3][1] = (bottom + top) / (bottom - top);
    result[3][2] = zNear / (zNear - zFar);
    result[3][3] = 1.f;
    return result;
}
}