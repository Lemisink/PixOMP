#include "executor/auto_executor.h"

#include <omp.h>

namespace img::detail {
    
void AutoExecutor::for_range_impl(size_t n, FunctionRef<void(size_t)> fn) {
    for_range_tid_impl(n, [&](size_t i, size_t) { fn(i); });
}

void AutoExecutor::for_range_tid_impl(size_t n, FunctionRef<void(size_t, size_t)> fn) {
    if (opt_.threads > 0) omp_set_num_threads(opt_.threads);
    omp_set_schedule(opt_.schedule == ScheduleType::Static ? omp_sched_static : omp_sched_dynamic,
                     opt_.chunk);
    #pragma omp parallel for schedule(runtime)
    for (size_t i = 0; i < n; i++) {
        fn(i, static_cast<size_t>(omp_get_thread_num()));
    }
}

size_t AutoExecutor::thread_count() const noexcept {
    if (opt_.threads > 0) return static_cast<size_t>(opt_.threads);
    return static_cast<size_t>(omp_get_max_threads());
}

}
