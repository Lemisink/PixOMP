#include "operation/resize/bilinear_resize.h"

#include <cmath>
#include <vector>
#include <algorithm>

#include "convertor/color_math.h"

namespace img::detail {

namespace {

template <typename ImageT>
void apply_resize(ImageT& img, ResizeOption& opt, IExecutor& exec) {
    const size_t src_w = img.width;
    const size_t src_h = img.height;
    const size_t dst_w = static_cast<size_t>(opt.width);
    const size_t dst_h = static_cast<size_t>(opt.height);
    const float scaleX = static_cast<float>(src_w) / static_cast<float>(dst_w);
    const float scaleY = static_cast<float>(src_h) / static_cast<float>(dst_h);

    ImageT out;
    out.height = static_cast<uint32_t>(dst_h);
    out.width = static_cast<uint32_t>(dst_w);
    out.data.resize(dst_h * dst_w);

    std::vector<float> all_srcX(dst_w);
    std::vector<float> all_srcY(dst_h);
    for (size_t x = 0; x < dst_w; x++) {
        const float src_x = (static_cast<float>(x) + 0.5f) * scaleX - 0.5f;
        all_srcX[x] = std::clamp<float>(src_x, 0.0f, static_cast<float>(src_w - 1));
    }
    for (size_t y = 0; y < dst_h; y++) {
        const float src_y = (static_cast<float>(y) + 0.5f) * scaleY - 0.5f;
        all_srcY[y] = std::clamp<float>(src_y, 0.0f, static_cast<float>(src_h - 1));
    }

    exec.for_range(dst_h, [&](size_t y) {
        const size_t y0 = std::floor(all_srcY[y]);
        const size_t y1 = std::min(y0 + 1, src_h - 1);
        const float dy = all_srcY[y] - y0;

        const size_t y0_offset = y0 * src_w;
        const size_t y1_offset = y1 * src_w;
        const size_t dst_y_offset = y * dst_w;

        for (size_t x = 0; x < dst_w; x++) {
            const size_t x0 = std::floor(all_srcX[x]);
            const size_t x1 = std::min(x0 + 1, src_w - 1);
            const float dx = all_srcX[x] - x0;

            out.data[dst_y_offset + x] =
                (1 - dx) * (1 - dy) * img.data[y0_offset + x0] +
                dx * (1 - dy) * img.data[y0_offset + x1] +
                (1 - dx) * dy * img.data[y1_offset + x0] +
                dx * dy * img.data[y1_offset + x1];
        }
    });

    img = std::move(out);
}

}

void BilinearResize::apply(ImageAny& img, IExecutor& exec) {
    std::visit([&](auto& concrete) {
        apply_resize(concrete, op_, exec);
    }, img);
}

}
