#pragma once

#include "convertor/image_convert.h"
#include "image.h"

namespace img::detail {

class ToYCb final : public Convertor<ImageYCb> {
    public:
        using Convertor::Convertor;
    private:
        ImageYCb from(const ImageRGB& img) override;
        ImageYCb from(const ImageGray& img) override;
        ImageYCb from(const ImageYCb& img) override;
};

}
