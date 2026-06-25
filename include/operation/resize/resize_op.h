#pragma once

#include "operation/operation.h"

namespace img::detail {

class ResizeOp : public IOperation {
    protected:
        ResizeOption op_;
    public:
        explicit ResizeOp(ResizeOption op) : op_(op) {};
};

}
