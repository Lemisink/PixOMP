#include "write_image.h"

#include <fstream>

namespace img {

namespace {

template <typename T>
bool write_bytes(std::ofstream& out, const std::vector<T>& data) {
    if (data.empty()) return static_cast<bool>(out);
    out.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size() * sizeof(T)));
    return static_cast<bool>(out);
}

}

WriteImageResult write(const ImageFile& image, std::string_view image_name) {
    std::ofstream out(std::string(image_name), std::ios::binary);
    if (!out) {
        return std::unexpected(WriteImageError::OpenFailed);
    }

    auto write_header = [&](const char* magic, uint32_t width, uint32_t height) {
        out << magic << "\n" << width << " " << height << "\n"
            << image.max_value << "\n";
    };

    switch (image.format) {
        case ImageType::P6: {
            const auto* rgb = std::get_if<ImageRGB>(&image.image);
            if (!rgb) return std::unexpected(WriteImageError::UnsupportedFormat);
            write_header("P6", rgb->width, rgb->height);
            if (!write_bytes(out, rgb->data)) {
                return std::unexpected(WriteImageError::WriteFailed);
            }
            return {};
        }
        case ImageType::P5: {
            const auto* gray = std::get_if<ImageGray>(&image.image);
            if (!gray) return std::unexpected(WriteImageError::UnsupportedFormat);
            write_header("P5", gray->width, gray->height);
            if (!write_bytes(out, gray->data)) {
                return std::unexpected(WriteImageError::WriteFailed);
            }
            return {};
        }
    }
    return std::unexpected(WriteImageError::UnsupportedFormat);
}

}
