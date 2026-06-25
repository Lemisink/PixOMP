#pragma once

#include <array>
#include <memory>
#include <expected>
#include <optional>

#include "image.h"
#include "config.h"

namespace img {

enum class ExecPolicy {
    Single,
    Omp,
    Manual
};

enum class ScheduleType {
    Static,
    Dynamic
};

enum class ProcessError {
    InvalidCoef,
    UnsupportedFormat
};

enum class ResizePolicy {
    Nearest,
    Bilinear,
    Bicubic
};

struct ExecOption {
    ExecPolicy policy = ExecPolicy::Single;
    ScheduleType schedule = ScheduleType::Static;
    int threads = 0;
    int chunk = 0;
};

struct ContrastOptions {
    double coef = 0.0;
};

struct ResizeOption {
    int width = 0;
    int height = 0;
    ResizePolicy policy = ResizePolicy::Bilinear;
};

struct ProcessOption {
    ExecOption exec{};
    std::optional<ContrastOptions> contrast;
    std::optional<ResizeOption> resize;
};

std::expected<void, ProcessError> process(ImageFile& image, const ProcessOption& option);

}
