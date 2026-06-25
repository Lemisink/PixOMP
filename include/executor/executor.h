#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <memory>

#include "image_process.h"

namespace img::detail {

template <typename>
class FunctionRef;

template <typename R, typename... Args>
class FunctionRef<R(Args...)> {
    public:
        template <typename F>
        FunctionRef(F&& f) noexcept
            : obj_(static_cast<void*>(std::addressof(f))) {
                cb_ = [](void* obj, Args... args) -> R {
                    return (*static_cast<std::remove_reference_t<F>*>(obj))(std::forward<Args>(args)...);
                };
            }

        R operator()(Args... args) const {
            return cb_(obj_, std::forward<Args>(args)...);
        }

    private:
        void* obj_;
        R (*cb_)(void*, Args...);
};

class IExecutor {
    public:
        explicit IExecutor(const ExecOption& opt) noexcept : opt_(opt) {}
        virtual ~IExecutor() = default;
        template <typename F>
        void for_range(size_t n, F&& fn) {
            for_range_impl(n, FunctionRef<void(size_t)>(fn));
        }
        template <typename F>
        void for_range_tid(size_t n, F&& fn) {
            for_range_tid_impl(n, FunctionRef<void(size_t, size_t)>(fn));
        }
        virtual size_t thread_count() const noexcept = 0;
    protected:
        virtual void for_range_impl(size_t n, FunctionRef<void(size_t)> fn) = 0;
        virtual void for_range_tid_impl(size_t n, FunctionRef<void(size_t, size_t)> fn) = 0;
        ExecOption opt_;
};

}
