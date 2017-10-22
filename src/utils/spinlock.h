#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include <atomic>

namespace utils {

class spinlock {
public:
    spinlock() = default;
    spinlock(const spinlock&) = delete;
    spinlock& operator=(const spinlock&) = delete;

    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire))
            ;
    }

    void unlock() { flag_.clear(std::memory_order_release); }

private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

}  // namespace utils
#endif  // _SPINLOCK_H_
