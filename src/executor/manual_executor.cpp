#include "executor/manual_executor.h"

#include <algorithm>
#include <omp.h>

#include "image_process.h"

namespace img::detail {
    
void ManualExecutor::for_range_impl(size_t n, FunctionRef<void(size_t)> fn) {
    for_range_tid_impl(n, [&](size_t i, size_t) { fn(i); });
}

void ManualExecutor::for_range_tid_impl(size_t n, FunctionRef<void(size_t, size_t)> fn) {
    size_t next = 0;
    if (opt_.threads > 0) omp_set_num_threads(opt_.threads);
    #pragma omp parallel 
    {
    const int total = omp_get_num_threads();
    const int tid = omp_get_thread_num();
    const int chunk = (opt_.chunk > 0) ? opt_.chunk
                                            : (n + total) / total;
    switch (opt_.schedule) {
    case ScheduleType::Static: {
        int chunk_count = (n + chunk - 1) / chunk;

        for (int i = tid; i < chunk_count; i += total) {
            size_t begin = static_cast<size_t>(i) * static_cast<size_t>(chunk);
            size_t end = std::min(static_cast<size_t>(n), begin + static_cast<size_t>(chunk));
            for (size_t j = begin; j < end; j++) {
                fn(j, static_cast<size_t>(tid));
            }
        }
        break;
    }
    case ScheduleType::Dynamic: {
        auto next_begin = [&]() {
            size_t b;
            #pragma omp atomic capture
            {
            b = next;
            next += chunk;
            }
            return b;
        };

        for (size_t begin = next_begin(); begin < n; begin = next_begin()) {
            size_t end = std::min(static_cast<size_t>(n), begin + static_cast<size_t>(chunk));
            for (size_t i = begin; i < end; i++) {
                fn(i, static_cast<size_t>(tid));
            }
        }
        break;
    }
    }
    }
}

size_t ManualExecutor::thread_count() const noexcept {
    if (opt_.threads > 0) return static_cast<size_t>(opt_.threads);
    return static_cast<size_t>(omp_get_max_threads());
}

}
