#pragma once
#include "vkg/base/vk_headers.hpp"
#include <span>
#include <cstring>

namespace vkg {
auto make(vk::Device device, const std::string &filename) -> vk::UniqueShaderModule;
auto make(vk::Device device, std::span<const uint32_t> opcodes) -> vk::UniqueShaderModule;

class SpcializationMaker {
public:
    template<typename... Args>
    auto sp(Args &&...spValues) {
        sp_(std::forward<Args>(spValues)...);
        specializationInfo_ = {uint32_t(spentries.size()), spentries.data(), spdata.size(), spdata.data()};
    }
    auto specializationInfo() const -> vk::SpecializationInfo;

private:
    auto sp_() {}
    template<typename T, typename... Args>
    auto sp_(T data, Args &&...rest) {
        sp_(data);
        sp_(rest...);
    }
    template<typename T>
    auto sp_(T &&data) {
        auto size = sizeof(T);
        spentries.emplace_back(spentries.size(), spoffset, size);
        auto *p = reinterpret_cast<std::byte *>(&data);
        spdata.reserve(spdata.size() + size);
        for(size_t i = 0; i < size; i++)
            spdata.emplace_back(*(p + i));
        spoffset += uint32_t(size);
    }

protected:
    uint32_t spoffset{0};
    std::vector<vk::SpecializationMapEntry> spentries;
    std::vector<std::byte> spdata;
    vk::SpecializationInfo specializationInfo_;
};

class Shader: public SpcializationMaker {
public:
    static auto readShaderFile(const std::string &filename) -> std::vector<uint32_t>;

    Shader() = default;

    template<typename... Args>
    explicit Shader(std::span<const uint32_t> opcodes, Args &&...spValues): opcodesSpan{opcodes} {
        sp(std::forward<Args>(spValues)...);
    }
    template<typename... Args>
    explicit Shader(const std::string &filename, Args &&...spValues) {
        sp(std::forward<Args>(spValues)...);
        opcodesVec = readShaderFile(filename);
        opcodesSpan = opcodesVec;
    }
    Shader(std::span<const uint32_t> opcodes, const vk::SpecializationInfo &specializationInfo): opcodesSpan{opcodes} {
        specializationInfo_ = specializationInfo;
    }
    Shader(std::span<const uint32_t> opcodes, vk::SpecializationInfo &specializationInfo): opcodesSpan{opcodes} {
        specializationInfo_ = specializationInfo;
    }
    Shader(const std::string &filename, const vk::SpecializationInfo &specializationInfo) {
        specializationInfo_ = specializationInfo;
        opcodesVec = readShaderFile(filename);
        opcodesSpan = opcodesVec;
    }
    Shader(const std::string &filename, vk::SpecializationInfo &specializationInfo) {
        specializationInfo_ = specializationInfo;
        opcodesVec = readShaderFile(filename);
        opcodesSpan = opcodesVec;
    }

    auto make(vk::Device) -> void;
    auto shaderModule() const -> vk::ShaderModule;

    auto empty() const -> bool;

private:
    std::span<const uint32_t> opcodesSpan;
    std::vector<uint32_t> opcodesVec;
    vk::UniqueShaderModule shaderModule_;
};
}
