#pragma once

#include <expected>
#include <string>

#include "image.h"

namespace img {

enum class WriteImageError {
    OpenFailed,
    UnsupportedFormat,
    WriteFailed
};

using WriteImageResult = std::expected<void, WriteImageError>;

WriteImageResult write(const ImageFile& image, const std::string_view image_name);

}
