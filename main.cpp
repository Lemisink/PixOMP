#include <iostream>
#include <omp.h>
#include <stdexcept>
#include <string>

#include "config.h"
#include "include/image_process.h"
#include "include/read_image.h"
#include "include/write_image.h"
#include "CLI/parser.h"

template<typename T> T from_string(const std::string& v);

template<>
ScheduleType from_string<ScheduleType>(const std::string& v) {
  if (v == "static") return ScheduleType::Static;
  else if (v == "dynamic") return ScheduleType::Dynamic;
  else throw std::runtime_error("Invalid schedule type: " + v);
}

namespace {

enum class OperationMode {
    Contrast,
    Resize,
    Both
};

OperationMode parse_operation(const std::string& value) {
    if (value == "contrast") return OperationMode::Contrast;
    if (value == "resize") return OperationMode::Resize;
    if (value == "both") return OperationMode::Both;
    throw std::runtime_error("Invalid operation: " + value);
}

img::ResizePolicy parse_resize_policy(const std::string& value) {
    if (value == "nearest") return img::ResizePolicy::Nearest;
    if (value == "bilinear") return img::ResizePolicy::Bilinear;
    if (value == "bicubic") return img::ResizePolicy::Bicubic;
    throw std::runtime_error("Invalid resize policy: " + value);
}

img::ExecPolicy parse_exec_policy(int value) {
    switch (value) {
        case 1:
            return img::ExecPolicy::Single;
        case 2:
            return img::ExecPolicy::Omp;
        case 3:
            return img::ExecPolicy::Manual;
        default:
            throw std::runtime_error("Invalid realization: " + std::to_string(value));
    }
}

img::ScheduleType to_img_schedule(::ScheduleType schedule) {
    return schedule == ::ScheduleType::Static
               ? img::ScheduleType::Static
               : img::ScheduleType::Dynamic;
}

bool needs_contrast(OperationMode operation) {
    return operation == OperationMode::Contrast || operation == OperationMode::Both;
}

bool needs_resize(OperationMode operation) {
    return operation == OperationMode::Resize || operation == OperationMode::Both;
}

}

int main(int argc, char** argv) {
    Options<std::string> opt_in("--in", "-i");
    Options<std::string> opt_out("--out", "-o");
    Options<std::string> opt_operation("--operation");
    opt_operation.value = "contrast";
    Options<double> opt_coef("--coef", "-c");
    opt_coef.value = 0.0;
    Options<int> opt_resize_width("--resize_width");
    opt_resize_width.value = 0;
    Options<int> opt_resize_height("--resize_height");
    opt_resize_height.value = 0;
    Options<std::string> opt_resize_policy("--resize_policy");
    opt_resize_policy.value = "bilinear";
    Options<int> opt_realization("--realization", "-r");
    opt_realization.value = 1;
    Options<int> opt_threads("--num_threads", "-t");
    opt_threads.value = 0;
    Options<int> opt_chunk("--chunk_size");
    opt_chunk.value = 0;
    Options<ScheduleType> opt_schedule("--schedule", "-s");
    opt_schedule.value = ScheduleType::Static;
    Options<int> opt_benchmark("--benchmark", "-b");
    opt_benchmark.value = 1;

    Parser<Options<std::string>, Options<std::string>, Options<std::string>,
           Options<double>, Options<int>, Options<int>, Options<std::string>,
           Options<int>, Options<int>, Options<int>, Options<ScheduleType>, Options<int>>
        parser(opt_in, opt_out, opt_operation, opt_coef, opt_resize_width,
               opt_resize_height, opt_resize_policy, opt_realization, opt_threads,
               opt_chunk, opt_schedule, opt_benchmark);

    Config config{};
    OperationMode operation_mode;
    img::ResizePolicy resize_policy;
    try {
        parser.pars(argc, const_cast<const char**>(argv));

        config.in_name = parser.get<std::string, "--in">();
        config.out_name = parser.get<std::string, "--out">();
        config.operation = parser.get<std::string, "--operation">();
        config.coef = parser.get<double, "--coef">();
        config.resize_width = parser.get<int, "--resize_width">();
        config.resize_height = parser.get<int, "--resize_height">();
        config.resize_policy = parser.get<std::string, "--resize_policy">();
        config.realization = parser.get<int, "--realization">();
        config.threads = parser.get<int, "--num_threads">();
        config.chunk = parser.get<int, "--chunk_size">();
        config.schedule_type = parser.get<ScheduleType, "--schedule">();

        if (config.in_name.empty()) {
            throw std::runtime_error("Missing required option: --in");
        }
        if (config.out_name.empty()) {
            throw std::runtime_error("Missing required option: --out");
        }

        operation_mode = parse_operation(config.operation);
        resize_policy = parse_resize_policy(config.resize_policy);
        parse_exec_policy(config.realization);

        if (needs_resize(operation_mode)
            && (config.resize_width <= 0 || config.resize_height <= 0)) {
            throw std::runtime_error(
                "Resize requires positive --resize_width and --resize_height");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    bool benchmark_mode = parser.get<int, "--benchmark">() != 0;
    auto img = img::read(config.in_name);
    if (!img) {
        std::cerr << "read error" << std::endl;
        return 1;
    }

    img::ProcessOption option{};
    option.exec.policy = parse_exec_policy(config.realization);
    option.exec.schedule = to_img_schedule(config.schedule_type);
    option.exec.threads = config.threads;
    option.exec.chunk = config.chunk;
    if (needs_contrast(operation_mode)) {
        option.contrast = img::ContrastOptions{config.coef};
    }
    if (needs_resize(operation_mode)) {
        option.resize = img::ResizeOption{
            .width = config.resize_width,
            .height = config.resize_height,
            .policy = resize_policy
        };
    }

    if (benchmark_mode) {
        constexpr int WARMUP_RUNS = 5;
        constexpr int BENCHMARK_RUNS = 50;

        for (int i = 0; i < WARMUP_RUNS; i++) {
            auto img_copy = img::read(config.in_name);
            if (!img_copy) {
                std::cerr << "read error" << std::endl;
                return 1;
            }
            auto result = img::process(*img_copy, option);
            if (!result) {
                std::cerr << "process error" << std::endl;
                return 1;
            }
        }

        double total_time = 0.0;
        for (int i = 0; i < BENCHMARK_RUNS; i++) {
            auto img_copy = img::read(config.in_name);
            if (!img_copy) {
                std::cerr << "read error" << std::endl;
                return 1;
            }

            double t0 = omp_get_wtime();
            auto process_result = img::process(*img_copy, option);
            double t1 = omp_get_wtime();

            if (!process_result) {
                std::cerr << "process error" << std::endl;
                return 1;
            }

            total_time += (t1 - t0);

            if (i == BENCHMARK_RUNS - 1) {
                *img = std::move(*img_copy);
            }
        }

        double avg_time_ms = (total_time / BENCHMARK_RUNS) * 1000.0;
        std::cout << "Average process time over " << BENCHMARK_RUNS << " runs: "
                  << avg_time_ms << " ms" << std::endl;
        std::cout << "Total time: " << total_time * 1000.0 << " ms" << std::endl;
    } else {
        double t0 = omp_get_wtime();
        auto process_result = img::process(*img, option);
        double t1 = omp_get_wtime();

        std::cout << "Process time: " << (t1 - t0) * 1000.0 << " ms" << std::endl;

        if (!process_result) {
            std::cerr << "process error" << std::endl;
            return 1;
        }
    }

    auto write_result = img::write(*img, config.out_name);
    if (!write_result) {
        std::cerr << "write error" << std::endl;
        return 1;
    }
}
