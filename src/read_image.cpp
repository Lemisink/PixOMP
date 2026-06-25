#include "read_image.h"

#include <string>
#include <fstream>

namespace {

using bstr = std::basic_string<uint8_t>;

bstr next_token(std::istream& image) {
    bstr token;
    while (true) {
        uint8_t ch = image.get();
        if (ch == '#') {
            while (image && image.get() != '\n') {}
        } else if (!std::isspace(ch)) {
            token.push_back(ch);
            break;
        }
    }

    while (true) {
        uint8_t ch = image.get();
        if (std::isspace(ch)) {
            break;
        } else {
            token.push_back(ch);
        }
    }

    return token;
}

uint32_t bstr2int(const bstr& bstr) {
    uint32_t ret = 0;
    for (uint8_t ch : bstr) {
        ret = ret * 10 + (ch - '0');
    }
    return ret;
}

}

namespace img {

ReadImageResult read(std::string_view image_name) {
    std::ifstream image(std::string(image_name), std::ios::binary);
    if (!image) {
        return std::unexpected(ReadImageError::OpenFailed);
    }

    const auto read_u32 = [&]() {
        return bstr2int(next_token(image));
    };

    const bstr header = next_token(image);
    const uint32_t width = read_u32(); 
    const uint32_t height = read_u32();
    const uint32_t max = read_u32();

    auto read_bytes = [&](auto& vec) {
        vec.resize(width * height);
        image.read(reinterpret_cast<char*>(vec.data()), vec.size() * sizeof(vec[0]));
        return static_cast<bool>(image);
    };

    if (header == bstr{'P', '6'} && max == 255) {
        ImageRGB image_out{width, height, {}};
        if (!read_bytes(image_out.data)) {
            return std::unexpected(ReadImageError::ReadFailed);
        }
        return ImageFile{std::move(image_out), ImageType::P6, max};
    } else if (header == bstr{'P', '5'} && max == 255) {
        ImageGray image_out{width, height, {}};
        if (!read_bytes(image_out.data)) {
            return std::unexpected(ReadImageError::ReadFailed);
        }
        return ImageFile{std::move(image_out), ImageType::P5, max};
    } else {
        return std::unexpected(ReadImageError::UnsupportedFormat);
    }
}

}
