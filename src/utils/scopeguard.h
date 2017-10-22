#ifndef _SCOPEGUARD_H_
#define _SCOPEGUARD_H_

namespace utils {

template <typename Func>
struct scope_guard {
    explicit scope_guard(Func&& on_exit) : on_exit_(std::move(on_exit)), enabled_(true) {}

    scope_guard(scope_guard const&) = delete;
    scope_guard& operator=(scope_guard const&) = delete;

    scope_guard(scope_guard&& other) : on_exit_(std::move(other.on_exit_)), enabled_(other.enabled_) {
        other.dismiss();
    }

    ~scope_guard() noexcept {
        if (enabled_)
            on_exit_();
    }

    void dismiss() { enabled_ = false; }

private:
    Func on_exit_;
    bool enabled_;
};

template <typename Func>
auto make_guard(Func&& f) -> scope_guard<Func> {
    return scope_guard<Func>(std::forward<Func>(f));
}

}  // namespace utils

#endif  // _SCOPEGUARD_H_
