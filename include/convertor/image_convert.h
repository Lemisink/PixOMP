#pragma once

#include <variant>

#include "image.h"
#include "executor/executor.h"

namespace img::detail {

template <typename Out>
class Convertor {
    public:
        explicit Convertor(IExecutor& exec) : exec_(exec) {}
        Out convert(const ImageRGB& img) { return from(img); }
        Out convert(const ImageGray& img) { return from(img); }
        Out convert(const ImageYCb& img) { return from(img); }
        Out convert(const ImageAny& img) {
            return std::visit([this](const auto& v) { return from(v); }, img);
        }
    protected:
        virtual Out from(const ImageRGB& img) = 0;
        virtual Out from(const ImageGray& img) = 0;
        virtual Out from(const ImageYCb& img) = 0;
        IExecutor& exec_;
};

}
