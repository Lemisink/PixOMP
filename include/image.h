#pragma once

#include <vector>
#include <cstdint>
#include <variant>
#include <ostream>
#include <string>
#include <type_traits>

enum class ImageType {
    P5,
    P6
};

enum class  ImageKind {
    Rgb,
    Gray,
    YCb
};

struct RGB {
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

struct YCbCr {
    float Y;
    float Cb;
    float Cr;
};

struct Gray {
    uint8_t gray;
};

struct ImageRGB {
    uint32_t width;
    uint32_t height;
    std::vector<RGB> data;
};

struct ImageYCb {
    uint32_t width;
    uint32_t height;
    std::vector<YCbCr> data;
};

struct ImageGray {
    uint32_t width;
    uint32_t height;
    std::vector<Gray> data;
};

using ImageAny = std::variant<ImageRGB, ImageGray, ImageYCb>;

struct ImageFile {
    ImageAny image;
    ImageType format = ImageType::P6;
    uint32_t max_value = 255;
};

inline ImageKind kind_of(const ImageAny& img) {
    return std::visit([](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, ImageRGB>) {
            return ImageKind::Rgb;
        } else if constexpr (std::is_same_v<T, ImageGray>) {
            return ImageKind::Gray;
        } else if constexpr (std::is_same_v<T, ImageYCb>) {
            return ImageKind::YCb;
        }
    }, img);
}

inline const char* to_string(ImageKind kind) noexcept {
    switch (kind) {
        case ImageKind::Rgb: return "RGB";
        case ImageKind::Gray: return "Gray";
        case ImageKind::YCb: return "YCb";
    }
}
