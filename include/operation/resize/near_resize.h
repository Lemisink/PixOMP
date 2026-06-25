#include "resize_op.h"

namespace img::detail {

class NearResize : public ResizeOp {
    public:
        using ResizeOp::ResizeOp;
        void apply(ImageAny& img, IExecutor& exec) override;
};

}
