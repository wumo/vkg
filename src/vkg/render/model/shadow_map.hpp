#pragma once

namespace vkg {
class ShadowMap {
public:
  auto isEnabled() const -> bool;
  auto enable(bool enabled) -> void;

private:
  bool enabled_{false};
};
}
