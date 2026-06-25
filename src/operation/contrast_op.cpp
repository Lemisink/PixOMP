#include "operation/contrast_op.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <vector>

#include "convertor/color_math.h"

namespace img::detail {

namespace {

struct alignas(64) PaddedHist {
    std::array<int, 256> data{};
};

// Округление как в оригинальном коде (+ 0.5f)
inline int y_index(float value) noexcept {
    return std::clamp<int>(static_cast<int>(value + 0.5f), 0, 255);
}

inline void merge_histograms(std::array<int, 256>& hist, const std::vector<PaddedHist>& locals) {
    const size_t threads = locals.size();
    for (size_t t = 0; t < threads; t++) {
        for (size_t i = 0; i < hist.size(); i++) {
            hist[i] += locals[t].data[i];
        }
    }
}

std::array<int, 256> hist_from_ycb(const ImageYCb& img, IExecutor& exec) {
    const size_t threads = std::max<size_t>(1, exec.thread_count());
    std::vector<PaddedHist> locals(threads);
    exec.for_range_tid(img.data.size(), [&](size_t i, size_t tid) {
        auto& local = locals[tid].data;
        local[y_index(img.data[i].Y)]++;
    });
    std::array<int, 256> hist{};
    merge_histograms(hist, locals);
    return hist;
}

std::array<int, 256> hist_from_rgb(const ImageRGB& img, IExecutor& exec) {
    const size_t threads = std::max<size_t>(1, exec.thread_count());
    std::vector<PaddedHist> locals(threads);
    exec.for_range_tid(img.data.size(), [&](size_t i, size_t tid) {
        auto& local = locals[tid].data;
        local[y_index(y_from_rgb(img.data[i]))]++;
    });
    std::array<int, 256> hist{};
    merge_histograms(hist, locals);
    return hist;
}

std::array<int, 256> hist_from_gray(const ImageGray& img, IExecutor& exec) {
    const size_t threads = std::max<size_t>(1, exec.thread_count());
    std::vector<PaddedHist> locals(threads);
    exec.for_range_tid(img.data.size(), [&](size_t i, size_t tid) {
        auto& local = locals[tid].data;
        local[img.data[i].gray]++;
    });
    std::array<int, 256> hist{};
    merge_histograms(hist, locals);
    return hist;
}

void low_and_high(const std::array<int, 256>& hist, size_t count, double coef,
                  int& low, int& high) noexcept {
    const int cut = static_cast<int>(coef * static_cast<double>(count));
    low = 0;
    high = 255;

    int total = 0;
    while (high > 0 && total + hist[high - 1] <= cut) {
        total += hist[--high];
    }

    total = 0;
    while (low < 255 && total + hist[low + 1] <= cut) {
        total += hist[++low];
    }
}

// МАКСИМАЛЬНО ТОЧНОЕ применение контраста без LUT
void apply_contrast(ImageYCb& img, const ContrastOptions& opt, IExecutor& exec) {
    auto hist = hist_from_ycb(img, exec);
    int low = 0;
    int high = 255;
    low_and_high(hist, img.data.size(), opt.coef, low, high);
    if (low >= high) return;

    // Вычисляем scale с максимальной точностью (double)
    const double scale = 255.0 / static_cast<double>(high - low);
    const double low_d = static_cast<double>(low);

    constexpr size_t chunk_size = 16384;
    const size_t total = img.data.size();
    const size_t num_chunks = (total + chunk_size - 1) / chunk_size;

    exec.for_range(num_chunks, [&](size_t chunk_id) {
        const size_t start = chunk_id * chunk_size;
        const size_t end = std::min(start + chunk_size, total);
        for (size_t i = start; i < end; i++) {
            auto& Y = img.data[i].Y;
            // СНАЧАЛА округляем Y до целого (как в старом коде через y_index)
            int y_int = y_index(Y);
            // ПОТОМ применяем преобразование контраста - ТОЧНО КАК В СТАРОМ КОДЕ
            int new_Y_int = static_cast<int>((y_int - low) * scale + 0.5);
            Y = static_cast<float>(std::clamp(new_Y_int, 0, 255));
        }
    });
}

// МАКСИМАЛЬНО ТОЧНОЕ применение контраста для RGB без LUT
void apply_contrast(ImageRGB& img, const ContrastOptions& opt, IExecutor& exec) {
    auto hist = hist_from_rgb(img, exec);
    int low = 0;
    int high = 255;
    low_and_high(hist, img.data.size(), opt.coef, low, high);
    if (low >= high) return;

    // Вычисляем scale с максимальной точностью (double)
    const double scale = 255.0 / static_cast<double>(high - low);
    const double low_d = static_cast<double>(low);

    constexpr size_t chunk_size = 16384;
    const size_t total = img.data.size();
    const size_t num_chunks = (total + chunk_size - 1) / chunk_size;

    exec.for_range(num_chunks, [&](size_t chunk_id) {
        const size_t start = chunk_id * chunk_size;
        const size_t end = std::min(start + chunk_size, total);
        for (size_t i = start; i < end; i++) {
            // Конвертируем в YCbCr
            auto ycb = rgb_to_ycb(img.data[i]);

            // СНАЧАЛА округляем Y до целого (как в старом коде через y_index)
            int y_int = y_index(ycb.Y);
            // ПОТОМ применяем преобразование контраста - ТОЧНО КАК В СТАРОМ КОДЕ
            int new_Y_int = static_cast<int>((y_int - low) * scale + 0.5);
            ycb.Y = static_cast<float>(std::clamp(new_Y_int, 0, 255));

            // Конвертируем обратно в RGB
            img.data[i] = ycb_to_rgb(ycb);
        }
    });
}

// МАКСИМАЛЬНО ТОЧНОЕ применение контраста для Gray без LUT
void apply_contrast(ImageGray& img, const ContrastOptions& opt, IExecutor& exec) {
    auto hist = hist_from_gray(img, exec);
    int low = 0;
    int high = 255;
    low_and_high(hist, img.data.size(), opt.coef, low, high);
    if (low >= high) return;

    // Вычисляем scale с максимальной точностью (double)
    const double scale = 255.0 / static_cast<double>(high - low);
    const double low_d = static_cast<double>(low);

    constexpr size_t chunk_size = 16384;
    const size_t total = img.data.size();
    const size_t num_chunks = (total + chunk_size - 1) / chunk_size;

    exec.for_range(num_chunks, [&](size_t chunk_id) {
        const size_t start = chunk_id * chunk_size;
        const size_t end = std::min(start + chunk_size, total);
        for (size_t i = start; i < end; i++) {
            // ТОЧНО КАК В СТАРОМ КОДЕ: округление через + 0.5
            int value = static_cast<int>((static_cast<int>(img.data[i].gray) - low) * scale + 0.5);
            img.data[i].gray = static_cast<uint8_t>(std::clamp(value, 0, 255));
        }
    });
}

}

void ContrastOp::apply(ImageAny& img, IExecutor& exec) {
    switch (kind_of(img)) {
        case ImageKind::Rgb: {
            apply_contrast(std::get<ImageRGB>(img), opt_, exec);
            break;
        }
        case ImageKind::YCb: {
            apply_contrast(std::get<ImageYCb>(img), opt_, exec);
            break;
        }
        case ImageKind::Gray: {
            apply_contrast(std::get<ImageGray>(img), opt_, exec);
            break;
        }
    }
}

}
