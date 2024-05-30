#include "benchmarking/data_collector.h"
#include <iostream>
#include <fstream>
#include <chrono>

namespace nvmeof {
namespace benchmarking {

DataCollector::DataCollector(const std::string& output_file)
    : output_file_(output_file) {}

void DataCollector::CollectData(const std::string& data_point) {
    auto timestamp = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(timestamp);

    std::ofstream file(output_file_, std::ios::app);
    if (file.is_open()) {
        file << std::ctime(&time) << " - " << data_point << std::endl;
        file.close();
    } else {
        std::cerr << "Error opening file: " << output_file_ << std::endl;
    }
}

}  // namespace benchmarking
}  // namespace nvmeof