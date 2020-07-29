#pragma once
#include "vkg/math/glm_common.hpp"

namespace vkg {

/**
 * define a new right-handed frame system (direction:-Z,right:+X,up:+Y)
 *
 * @param eye
 * @param center
 * @param up
 * @return
 */
auto viewMatrix(glm::vec3 eye, glm::vec3 center, glm::vec3 up) -> glm::mat4;

/**
 * define a perspective projection (direction:+Z,right:+X,up:-Y), x in [-1,1],
 * y in [-1,1], z in [0,1] that is the vulkan clipping volume.
 * @param fovy
 * @param aspect width/height
 * @param zNear
 * @param zFar
 * @return
 */
auto perspectiveMatrix(float fovy, float aspect, float zNear, float zFar) -> glm::mat4;

/**
 * define a orthographic  projection (direction:+Z,right:+X,up:-Y), x in [-1,1],
 * y in [-1,1], z in [0,1] that is the vulkan clipping volume.
 */
auto orthoMatrix(
  float left, float right, float bottom, float top, float zNear, float zFar) -> glm::mat4;
/**
 * define a orthographic  projection (direction:+Z,right:+X,up:-Y), x in [-1,1],
 * y in [-1,1], z in [0,1] that is the vulkan clipping volume.
 * @param with
 * @param height
 * @param zNear
 * @param zFar
 * @return
 */
auto orthoMatrix(float with, float height, float zNear, float zFar) -> glm::mat4;
}