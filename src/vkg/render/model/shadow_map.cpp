#include "shadow_map.hpp"
namespace vkg {
auto ShadowMap::isEnabled() const -> bool { return enabled_; }
auto ShadowMap::enable(bool enabled) -> void { enabled_ = enabled; }
}
