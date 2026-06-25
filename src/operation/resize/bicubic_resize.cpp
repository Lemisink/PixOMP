#include "operation/resize/bicubic_resize.h"

#include <cmath>
#include <vector>
#include <array>
#include <algorithm>

#include "convertor/color_math.h"

namespace img::detail {

namespace {

constexpr float kCatmullA = -0.5f;
constexpr float kMitchellB = 1.0f / 3.0f;
constexpr float kMitchellC = 1.0f / 3.0f;

inline float c_w_catmull(const float x) noexcept {
    const float abs_x = (x < 0.0f) ? -x : x;
    if (abs_x < 1.0f) {
        return (kCatmullA + 2.0f) * abs_x * abs_x * abs_x - (kCatmullA + 3.0f) * abs_x * abs_x + 1.0f;
    } else if (abs_x < 2.0f) {
        return kCatmullA * abs_x * abs_x * abs_x - 5.0f * kCatmullA * abs_x * abs_x + 8.0f * kCatmullA * abs_x - 4.0f * kCatmullA;
    } else {
        return 0.0f;
    }
}

inline float c_w_mitchell(const float x) noexcept {
    const float abs_x = (x < 0.0f) ? -x : x;
    if (abs_x < 1.0f) {
        return (1.0f / 6.0f)
            * ((12.0f - 9.0f * kMitchellB - 6.0f * kMitchellC) * abs_x * abs_x * abs_x
                + (-18.0f + 12.0f * kMitchellB + 6.0f * kMitchellC) * abs_x * abs_x
                + (6.0f - 2.0f * kMitchellB));
    } else if (abs_x < 2.0f) {
        return (1.0f / 6.0f)
            * ((-kMitchellB - 6.0f * kMitchellC) * abs_x * abs_x * abs_x
                + (6.0f * kMitchellB + 30.0f * kMitchellC) * abs_x * abs_x
                + (-12.0f * kMitchellB - 48.0f * kMitchellC) * abs_x
                + (8.0f * kMitchellB + 24.0f * kMitchellC));
    } else {
        return 0.0f;
    }
}

inline float c_w(const float x) noexcept {
    return c_w_mitchell(x);
}

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
        const float dy = all_srcY[y] - y0;
        std::array<float, 4> wx;
        std::array<float, 4> wy;
        wy[0] = c_w(dy + 1);
        wy[1] = c_w(dy);
        wy[2] = c_w(1 - dy);
        wy[3] = c_w(2 - dy);

        std::array<size_t, 4> y_offsets;
        for (int i = 0; i < 4; i++) {
            const int y_coord = static_cast<int>(y0) - 1 + i;
            const size_t y_clamped = std::clamp(y_coord, 0, static_cast<int>(src_h - 1));
            y_offsets[i] = y_clamped * src_w;
        }

        const size_t dst_y_offset = y * dst_w;

        for (size_t x = 0; x < dst_w; x++) {
            const size_t x0 = std::floor(all_srcX[x]);
            const float dx = all_srcX[x] - x0;

            wx[0] = c_w(dx + 1);
            wx[1] = c_w(dx);
            wx[2] = c_w(1 - dx);
            wx[3] = c_w(2 - dx);

            std::array<size_t, 4> x_indices;
            for (int j = 0; j < 4; j++) {
                const int x_coord = static_cast<int>(x0) - 1 + j;
                x_indices[j] = std::clamp(x_coord, 0, static_cast<int>(src_w - 1));
            }

            auto result = img.data[y_offsets[0] + x_indices[0]] * 0.0f;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    result = result + img.data[y_offsets[i] + x_indices[j]] * (wx[j] * wy[i]);
                }
            }
            out.data[dst_y_offset + x] = result;
        }
    });

    img = std::move(out);
}

}

void BicubicResize::apply(ImageAny& img, IExecutor& exec) {
    std::visit([&](auto& concrete) {
        apply_resize(concrete, op_, exec);
    }, img);
}

}
