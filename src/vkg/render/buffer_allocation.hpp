#pragma once

#include "allocation.hpp"
#include "ranges.hpp"
#include "vkg/util/syntactic_sugar.hpp"
#include "vkg/base/resource/buffers.hpp"
#include <functional>
#include <span>
#include <utility>

namespace vkg {

using BufferAllocator = std::function<std::unique_ptr<Buffer>(
  Device &device, vk::DeviceSize sizeInBYtes, const std::string &name)>;

template<typename T>
class ContiguousAllocation {

public:
  ContiguousAllocation(
    const BufferAllocator &allocator, Device &device, uint32_t maxNum, std::string name)
    : maxNum_{maxNum}, count_{0}, name{std::move(name)} {
    buffer_ = allocator(device, maxNum * sizeof(T), this->name);
    device.name(buffer_->bufferInfo().buffer, this->name);
  }

  auto add(std::span<T> data) -> UIntRange {
    errorIf(count_ + data.size() >= maxNum_, "buffer ", name, " is full, max: ", maxNum_);
    auto offset = count_;
    buffer::upload(*buffer_, data.data(), data.size_bytes(), count_ * sizeof(T));
    count_ += uint32_t(data.size());
    return {offset, uint32_t(data.size())};
  }

  auto add(uint32_t num) -> UIntRange {
    errorIf(count_ + num >= maxNum_, "buffer ", name, " is full, max: ", maxNum_);
    auto offset = count_;
    count_ += num;
    return {offset, num};
  }

  auto update(UIntRange alloc, std::span<T> data) -> void {
    errorIf(
      data.size() > alloc.size && alloc.endExclusive() > count_, "update buffer ", name,
      " overflow: count=", count_, " alloc[start=", alloc.start, ", size=", alloc.size,
      "],data.size=", data.size());
    buffer::upload(*buffer_, data.data(), data.size_bytes(), alloc.start * sizeof(T));
  }

  auto count() const -> uint32_t { return count_; }
  auto bufferInfo() const -> BufferInfo { return buffer_->bufferInfo(); }

private:
  std::unique_ptr<Buffer> buffer_;
  const uint32_t maxNum_;
  uint32_t count_;
  std::string name;
};

template<typename T>
class RandomHostAllocation {
public:
  RandomHostAllocation(
    const BufferAllocator &allocator, Device &device, uint32_t maxNum, std::string name)
    : maxNum(maxNum), name{std::move(name)} {
    buffer_ = allocator(device, maxNum * sizeof(T), this->name);
    freeSlots.reserve(maxNum);
    for(int32_t i = maxNum; i > 0; --i)
      freeSlots.emplace_back(i - 1);
  }

  auto allocate() -> Allocation<T> {
    errorIf(freeSlots.empty(), "buffer ", name, " is full, max: ", maxNum);
    auto offset = freeSlots.back();
    freeSlots.pop_back();
    return {offset, buffer_->ptr<T>() + offset};
  }

  auto deallocate(Allocation<T> allocation) -> void {
    errorIf(
      allocation.ptr != buffer_->ptr<T>() + allocation.offset, "Invalid allocation");
    freeSlots.emplace_back(allocation.offset);
  }

  auto update(uint32_t offset, T data) -> void {
    errorIf(offset >= maxNum, "index is out of bounds buffer:", name, ", max: ", maxNum);
    buffer::updateSingle(*buffer_, data, offset * sizeof(T));
  }

  auto bufferInfo() const -> BufferInfo { return buffer_->bufferInfo(); }
  auto count() const { return uint32_t(maxNum - freeSlots.size()); }
  auto size() const { return count() * sizeof(T); }
  auto flush(vk::CommandBuffer cb) {}

private:
  std::unique_ptr<Buffer> buffer_;
  std::vector<uint32_t> freeSlots;
  const uint32_t maxNum;
  std::string name;
};

template<typename T>
class RandomHostAllocation2 {
public:
  RandomHostAllocation2(
    const BufferAllocator &allocator, Device &device, uint32_t maxNum, std::string name)
    : maxNum_(maxNum), name{std::move(name)} {
    buffer_ = allocator(device, maxNum * sizeof(T), this->name);
    hostBuffer.resize(maxNum_);
  }

  auto allocate() -> Allocation<T> {
    errorIf(count_ + 1 >= maxNum_, "buffer ", name, " is full, max: ", maxNum_);
    auto offset = count_;
    count_++;
    return {offset, hostBuffer.data() + offset};
  }

  auto count() const { return count_; }
  auto size() const { return count() * sizeof(T); }

  auto flush(vk::CommandBuffer cb) {
    if(count_ == 0) return;
    auto tStart = std::chrono::high_resolution_clock::now();
    println(name, "-> size:", size(), " count:", count_, "sizeof:", sizeof(T));
    memcpy(buffer_->ptr<T>(), hostBuffer.data(), size());
    auto tEnd = std::chrono::high_resolution_clock::now();
    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    println("flush ", name, " took ", tDiff, " ms");
  }

private:
  std::unique_ptr<Buffer> buffer_;
  std::vector<T> hostBuffer;
  const uint32_t maxNum_;
  uint32_t count_{0};

  std::string name;
};
}