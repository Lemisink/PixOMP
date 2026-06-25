#include "convertor/gray_convertor.h"

#include "executor/executor.h"
#include "convertor/color_math.h"

namespace img::detail {

ImageGray ToGray::from(const ImageGray& img) {
    return img;
}

ImageGray ToGray::from(const ImageRGB& img) {
    ImageGray out{img.width, img.height, {}};
    out.data.resize(img.data.size());
    exec_.for_range(out.data.size(), [&](size_t i) {
        out.data[i] = rgb_to_gray(img.data[i]);
    });
    return out;
}

ImageGray ToGray::from(const ImageYCb& img) {
    ImageGray out{img.width, img.height, {}};
    out.data.resize(img.data.size());
    exec_.for_range(out.data.size(), [&](size_t i) {
        out.data[i] = ycb_to_gray(img.data[i]);
    });
    return out;
}

}
