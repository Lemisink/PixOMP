#include "image_process.h"

#include <memory>

#include "executor/auto_executor.h"
#include "executor/manual_executor.h"
#include "executor/single_executor.h"
#include "operation/resize/bilinear_resize.h"
#include "operation/resize/bicubic_resize.h"
#include "operation/contrast_op.h"
#include "operation/resize/near_resize.h"

namespace img {

namespace {

std::unique_ptr<detail::IExecutor> make_executor(const ExecOption& opt) {
    using detail::AutoExecutor;
    using detail::ManualExecutor;
    using detail::SingleExecutor;
    switch (opt.policy) {
        case ExecPolicy::Single:
            return std::make_unique<SingleExecutor>(opt);
        case ExecPolicy::Omp:
            return std::make_unique<AutoExecutor>(opt);
        case ExecPolicy::Manual:
            return std::make_unique<ManualExecutor>(opt);
    }
    return nullptr;
}

}

std::expected<void, ProcessError> process(ImageFile& image, const ProcessOption& option) {
    if (option.contrast) {
        const double coef = option.contrast->coef;
        if (coef < 0.0 || coef >= 0.5) {
            return std::unexpected(ProcessError::InvalidCoef);
        }
    }

    auto exec = make_executor(option.exec);
    if (!exec) {
        return std::unexpected(ProcessError::UnsupportedFormat);
    }

    if (option.contrast) {
        detail::ContrastOp contrast(*option.contrast);
        contrast.apply(image.image, *exec);
    }

    if (option.resize) {
        switch (option.resize->policy) {
            case ResizePolicy::Nearest: {
                detail::NearResize resize(*option.resize);
                resize.apply(image.image, *exec);
                break;
            }
            case ResizePolicy::Bilinear: {
                detail::BilinearResize resize(*option.resize);
                resize.apply(image.image, *exec);
                break;
            }
            case ResizePolicy::Bicubic: {
                detail::BicubicResize resize(*option.resize);
                resize.apply(image.image, *exec);
                break;
            }
        }
    }

    return {};
}

}
