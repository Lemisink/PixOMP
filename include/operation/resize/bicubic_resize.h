#include "resize_op.h"

namespace img::detail {

class BicubicResize : public ResizeOp {
    public:
        using ResizeOp::ResizeOp;
        void apply(ImageAny& img, IExecutor& exec) override;
};

}
