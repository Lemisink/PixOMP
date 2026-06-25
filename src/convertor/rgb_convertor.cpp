#include "convertor/rgb_convertor.h"

#include "executor/executor.h"
#include "convertor/color_math.h"

namespace img::detail {

ImageRGB ToRGB::from(const ImageRGB& img) {
    return img;
}

ImageRGB ToRGB::from(const ImageYCb& img) {
    ImageRGB out{img.width, img.height, {}};
    out.data.resize(img.data.size());
    exec_.for_range(out.data.size(), [&](size_t i) {
        out.data[i] = ycb_to_rgb(img.data[i]);
    });
    return out;
}

ImageRGB ToRGB::from(const ImageGray& img) {
    ImageRGB out{img.width, img.height, {}};
    out.data.resize(img.data.size());
    exec_.for_range(out.data.size(), [&](size_t i) {
        out.data[i] = gray_to_rgb(img.data[i]);
    });
    return out;
}

}
