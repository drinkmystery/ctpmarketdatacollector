#include "global.h"

namespace global {

std::atomic<bool> need_reconnect = ATOMIC_FLAG_INIT;

}  // namespace global
