#pragma once

#include "executor/executor.h"

namespace img::detail {

class AutoExecutor final : public IExecutor {
    public:
        using IExecutor::IExecutor;
        size_t thread_count() const noexcept override;
    protected:
        void for_range_impl(size_t n, FunctionRef<void(size_t)> fn) override;
        void for_range_tid_impl(size_t n, FunctionRef<void(size_t, size_t)> fn) override;
};

} 
