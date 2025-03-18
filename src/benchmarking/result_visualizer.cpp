#include "../../include/benchmarking/result_visualizer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>

namespace nvmeof {
namespace benchmarking {

ResultVisualizer::ResultVisualizer(const std::string& input_file)
    : input_file_(input_file) {
    // Validate input file exists
    std::ifstream file(input_file_);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << input_file_ << std::endl;
    }
}

void ResultVisualizer::Visualize() {
    std::ifstream file(input_file_);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << input_file_ << std::endl;
        return;
    }

    // Read CSV header
    std::string header;
    std::getline(file, header);

    // Parse header to find column indices
    std::istringstream header_stream(header);
    std::vector<std::string> columns;
    std::string column;
    while (std::getline(header_stream, column, ',')) {
        columns.push_back(column);
    }

    // Find indices for timestamp, label, value, and units columns
    int timestamp_idx = -1, label_idx = -1, value_idx = -1, units_idx = -1;
    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i] == "Timestamp") timestamp_idx = i;
        else if (columns[i] == "Label") label_idx = i;
        else if (columns[i] == "Value") value_idx = i;
        else if (columns[i] == "Units") units_idx = i;
    }

    // If columns not found, return with a message
    if (timestamp_idx == -1 || label_idx == -1 || value_idx == -1) {
        std::cout << "No data points found or invalid CSV format." << std::endl;
        return;
    }

    // Read data lines
    std::vector<std::vector<std::string>> data_points;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream line_stream(line);
        std::vector<std::string> fields;
        std::string field;
        while (std::getline(line_stream, field, ',')) {
            fields.push_back(field);
        }
        
        // Skip invalid lines
        if (fields.size() <= static_cast<size_t>(std::max({timestamp_idx, label_idx, value_idx, units_idx}))) {
            continue;
        }
        
        data_points.push_back(fields);
    }

    file.close();

    if (data_points.empty()) {
        std::cout << "No data points found." << std::endl;
        return;
    }

    // Print benchmark results
    std::cout << "Benchmark Results:" << std::endl;
    std::cout << std::left << std::setw(25) << "Timestamp" 
              << std::setw(20) << "Data Point" 
              << std::setw(15) << "Value"
              << "Units" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (const auto& data_point : data_points) {
        std::cout << std::left 
                  << std::setw(25) << data_point[timestamp_idx] 
                  << std::setw(20) << data_point[label_idx]
                  << std::setw(15) << data_point[value_idx];
        
        if (units_idx != -1 && static_cast<size_t>(units_idx) < data_point.size()) {
            std::cout << data_point[units_idx];
        }
        
        std::cout << std::endl;
    }
}

}  // namespace benchmarking
}  // namespace nvmeof