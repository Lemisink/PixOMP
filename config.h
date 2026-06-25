#pragma once

#include <string>

enum class ScheduleType {
    Static,
    Dynamic
};

struct Config {
    std::string in_name;
    std::string out_name;
    std::string operation;
    std::string resize_policy;
    ScheduleType schedule_type;
    double coef;
    int realization;
    int threads;
    int chunk;
    int resize_width;
    int resize_height;
};
