#pragma once

#include <expected>
#include <string>

#include "image.h"

namespace img {

enum class ReadImageError {
    OpenFailed,
    UnsupportedFormat,
    ReadFailed
};

using ReadImageResult = std::expected<ImageFile, ReadImageError>;

ReadImageResult read(const std::string_view image_name);

}
