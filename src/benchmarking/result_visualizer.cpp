#include "benchmarking/result_visualizer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>

namespace nvmeof {
namespace benchmarking {

ResultVisualizer::ResultVisualizer(const std::string& input_file)
    : input_file_(input_file) {}

void ResultVisualizer::Visualize() {
    std::ifstream file(input_file_);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << input_file_ << std::endl;
        return;
    }

    std::vector<std::pair<std::string, std::string>> data_points;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string timestamp, data_point;
        if (std::getline(iss, timestamp, '-') && std::getline(iss, data_point)) {
            data_points.emplace_back(timestamp, data_point);
        }
    }

    file.close();

    if (data_points.empty()) {
        std::cout << "No data points found." << std::endl;
        return;
    }

    std::cout << "Benchmark Results:" << std::endl;
    std::cout << std::left << std::setw(25) << "Timestamp" << std::setw(20) << "Data Point" << std::endl;
    std::cout << std::string(45, '-') << std::endl;

    for (const auto& data_point : data_points) {
        std::cout << std::left << std::setw(25) << data_point.first << std::setw(20) << data_point.second << std::endl;
    }
}

}  // namespace benchmarking
}  // namespace nvmeof