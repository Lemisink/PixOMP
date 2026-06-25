#pragma once

#include "image.h"
#include "executor/executor.h"

namespace img::detail {

class IOperation {
    public:
        virtual ~IOperation() = default;
        virtual void apply(ImageAny& img, IExecutor& exec) = 0;
};

}
