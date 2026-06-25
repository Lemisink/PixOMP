#pragma once

#include "operation.h"

namespace img::detail {

class ContrastOp final : public IOperation {
    private:
        ContrastOptions opt_;
    public:
        explicit ContrastOp(ContrastOptions opt) : opt_(opt) {};
        void apply(ImageAny& img, IExecutor& exec) override;
};

}
