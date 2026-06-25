#pragma once

#include "convertor/image_convert.h"
#include "image.h"

namespace img::detail {

class ToRGB final : public Convertor<ImageRGB> {
    public:
        using Convertor::Convertor;
    private:
        ImageRGB from(const ImageRGB& img) override;
        ImageRGB from(const ImageYCb& img) override;
        ImageRGB from(const ImageGray& img) override;
};

}
