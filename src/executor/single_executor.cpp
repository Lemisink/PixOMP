#include "executor/single_executor.h"

namespace img::detail {
    
void SingleExecutor::for_range_impl(size_t n, FunctionRef<void(size_t)> fn) {
    for_range_tid_impl(n, [&](size_t i, size_t) { fn(i); });
}

void SingleExecutor::for_range_tid_impl(size_t n, FunctionRef<void(size_t, size_t)> fn) {
    for (size_t i = 0; i < n; i++) {
        fn(i, 0);
    }
}

size_t SingleExecutor::thread_count() const noexcept {
    return 1;
}

}
