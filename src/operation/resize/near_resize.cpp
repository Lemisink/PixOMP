#include "operation/resize/near_resize.h"

#include <vector>
#include <algorithm>
#include <cmath>

namespace img::detail {

namespace {

template <typename ImageT>
void apply_resize(ImageT& img, const ResizeOption& opt, IExecutor& exec) {
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

    std::vector<size_t> all_srcX(dst_w);
    std::vector<size_t> all_srcY(dst_h);
    for (size_t x = 0; x < dst_w; x++) {
        float srcX = (static_cast<float>(x) + 0.5f) * scaleX - 0.5f;
        const int src_x = static_cast<int>(srcX + 0.5f);
        all_srcX[x] = static_cast<size_t>(
            std::clamp<int>(src_x, 0, static_cast<int>(src_w - 1)));
    }
    for (size_t y = 0; y < dst_h; y++) {
        float srcY = (static_cast<float>(y) + 0.5f) * scaleY - 0.5f;
        const int src_y = static_cast<int>(srcY + 0.5f);
        all_srcY[y] = static_cast<size_t>(
            std::clamp<int>(src_y, 0, static_cast<int>(src_h - 1)));
    }

    exec.for_range(dst_h, [&](size_t y) {
        const size_t src_y_offset = all_srcY[y] * src_w;
        const size_t dst_y_offset = y * dst_w;
        for (size_t x = 0; x < dst_w; x++) {
            out.data[dst_y_offset + x] = img.data[src_y_offset + all_srcX[x]];
        }
    });

    img = std::move(out);
}

}

void NearResize::apply(ImageAny& img, IExecutor& exec) {
    std::visit([&](auto& concrete) {
        apply_resize(concrete, op_, exec);
    }, img);
}

}
