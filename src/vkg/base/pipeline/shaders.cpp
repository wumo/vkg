#include "shaders.hpp"
#include <fstream>
#include <string>
#include <vector>
#include "vkg/util/syntactic_sugar.hpp"
#include <spirv_glsl.hpp>

namespace vkg {
auto make(vk::Device device, const std::string &filename) -> vk::UniqueShaderModule {
  auto opcodes = Shader::readShaderFile(filename);
  return make(device, opcodes);
}

auto make(vk::Device device, std::span<const uint32_t> opcodes)
  -> vk::UniqueShaderModule {
  vk::ShaderModuleCreateInfo shaderModuleInfo;
  shaderModuleInfo.codeSize = opcodes.size_bytes();
  shaderModuleInfo.pCode = opcodes.data();
  return device.createShaderModuleUnique(shaderModuleInfo);
}

auto SpcializationMaker::specializationInfo() const -> vk::SpecializationInfo {
  return specializationInfo_;
}

auto Shader::readShaderFile(const std::string &filename) -> std::vector<uint32_t> {
  auto file = std::ifstream(filename, std::ios::ate | std::ios::binary);
  errorIf(!file.good(), "failed to read file!", filename);

  auto length = size_t(file.tellg());
  std::vector<uint32_t> opcodes;
  opcodes.resize(length / sizeof(uint32_t));
  file.seekg(0, std::ios::beg);
  file.read(reinterpret_cast<char *>(opcodes.data()), opcodes.size() * sizeof(uint32_t));
  file.close();
  return opcodes;
}

auto Shader::make(vk::Device device) -> void {
  shaderModule_ = vkg::make(device, opcodesSpan);
}
auto Shader::shaderModule() const -> vk::ShaderModule { return *shaderModule_; }
auto Shader::empty() const -> bool { return opcodesSpan.empty(); }

}