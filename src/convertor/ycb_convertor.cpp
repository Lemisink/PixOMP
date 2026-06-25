#include "convertor/ycb_convertor.h"

#include "executor/executor.h"
#include "convertor/color_math.h"

namespace img::detail {

ImageYCb ToYCb::from(const ImageRGB& img) {
    ImageYCb out{img.width, img.height, {}};
    out.data.resize(img.data.size());
    exec_.for_range(out.data.size(), [&](size_t i) {
        out.data[i] = rgb_to_ycb(img.data[i]); 
    });
    return out;
}

ImageYCb ToYCb::from(const ImageGray& img) {
    ImageYCb out{img.width, img.height, {}};
    out.data.resize(img.data.size());
    exec_.for_range(out.data.size(), [&](size_t i) {
        out.data[i] = gray_to_ycb(img.data[i]);
    });
    return out;
}

ImageYCb ToYCb::from(const ImageYCb& img) {
    return img;
}

}
