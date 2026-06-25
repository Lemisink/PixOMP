#pragma once

#include <algorithm>

#include "image.h"

namespace img::detail {

inline uint8_t clamp_u8(int value) noexcept {
    return static_cast<uint8_t>(std::clamp(value, 0, 255));
}

inline uint8_t clamp_u8(float value) noexcept {
    return clamp_u8(static_cast<int>(value + 0.5f));
}

struct GrayF {
    float v;
    operator Gray() const noexcept { return Gray{clamp_u8(v)}; }
};

struct RGBF {
    float r;
    float g;
    float b;
    operator RGB() const noexcept { return RGB{clamp_u8(r), clamp_u8(g), clamp_u8(b)}; }
};

inline float y_from_rgb(const RGB& rgb) noexcept {
    return 0.299f * rgb.R + 0.587f * rgb.G + 0.114f * rgb.B;
}

inline YCbCr rgb_to_ycb(const RGB& rgb) noexcept {
    return YCbCr{
        .Y = y_from_rgb(rgb),
        .Cb = -0.168736f * rgb.R - 0.331264f * rgb.G + 0.5f * rgb.B + 128.0f,
        .Cr = 0.5f * rgb.R - 0.418688f * rgb.G - 0.081312f * rgb.B + 128.0f
    };
}

inline RGB ycb_to_rgb(const YCbCr& ycb) noexcept {
    const float Cb_128 = ycb.Cb - 128.0f;
    const float Cr_128 = ycb.Cr - 128.0f;

    auto clamp_u8 = [](float value) {
        return static_cast<uint8_t>(std::clamp(value, 0.0f, 255.0f));
    };
    return RGB{
        .R = clamp_u8(ycb.Y + 1.402f * Cr_128),
        .G = clamp_u8(ycb.Y + 0.344136f * Cb_128 - 0.714136f * Cr_128),
        .B = clamp_u8(ycb.Y + 1.772f * Cb_128)
    };
}

inline YCbCr gray_to_ycb(const Gray& gray) noexcept {
    return YCbCr{
      .Y = static_cast<float>(gray.gray),
      .Cb = 128.0f,
      .Cr = 128.0f  
    };
}

inline RGB gray_to_rgb(const Gray& gray) noexcept {
    return RGB{gray.gray, gray.gray, gray.gray};
}

inline Gray rgb_to_gray(const RGB& rgb) noexcept {
    return Gray{static_cast<uint8_t>(y_from_rgb(rgb))};
}

inline Gray ycb_to_gray(const YCbCr& ycb) noexcept {
    return Gray{static_cast<uint8_t>(ycb.Y)};
}

inline Gray operator+(const Gray& a, const Gray& b) noexcept {
    return Gray{clamp_u8(static_cast<int>(a.gray) + static_cast<int>(b.gray))};
}

inline GrayF operator+(GrayF a, GrayF b) noexcept {
    return GrayF{a.v + b.v};
}

inline GrayF& operator+=(GrayF& a, GrayF b) noexcept {
    a.v += b.v;
    return a;
}

inline GrayF operator*(const Gray& a, float k) noexcept {
    return GrayF{static_cast<float>(a.gray) * k};
}

inline GrayF operator*(float k, const Gray& a) noexcept {
    return a * k;
}

inline RGB operator+(const RGB& a, const RGB& b) noexcept {
    return RGB{
        clamp_u8(static_cast<int>(a.R) + static_cast<int>(b.R)),
        clamp_u8(static_cast<int>(a.G) + static_cast<int>(b.G)),
        clamp_u8(static_cast<int>(a.B) + static_cast<int>(b.B))
    };
}

inline RGBF operator+(RGBF a, RGBF b) noexcept {
    return RGBF{a.r + b.r, a.g + b.g, a.b + b.b};
}

inline RGBF& operator+=(RGBF& a, RGBF b) noexcept {
    a.r += b.r;
    a.g += b.g;
    a.b += b.b;
    return a;
}

inline RGBF operator*(const RGB& a, float k) noexcept {
    return RGBF{static_cast<float>(a.R) * k,
                static_cast<float>(a.G) * k,
                static_cast<float>(a.B) * k};
}

inline RGBF operator*(float k, const RGB& a) noexcept {
    return a * k;
}

inline YCbCr operator+(const YCbCr& a, const YCbCr& b) noexcept {
    return YCbCr{a.Y + b.Y, a.Cb + b.Cb, a.Cr + b.Cr};
}

inline YCbCr operator*(const YCbCr& a, float k) noexcept {
    return YCbCr{a.Y * k, a.Cb * k, a.Cr * k};
}

inline YCbCr operator*(float k, const YCbCr& a) noexcept {
    return a * k;
}

}
