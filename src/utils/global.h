#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <atomic>

namespace global {

extern std::atomic<bool> need_reconnect;

}  // namespace global

#endif  // _GLOBAL_H_
