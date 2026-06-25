#pragma once

#include "convertor/image_convert.h"
#include "image.h"

namespace img::detail {

class ToGray final : public Convertor<ImageGray> {
    public:
        using Convertor::Convertor;
    private:
        ImageGray from(const ImageGray& img) override;
        ImageGray from(const ImageRGB& img) override;
        ImageGray from(const ImageYCb& img) override;
};

}
