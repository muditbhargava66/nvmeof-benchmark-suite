#include "../../include/benchmarking/data_collector.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cassert>

namespace nvmeof {
namespace benchmarking {

DataPoint::DataPoint(const std::string& label, double value, const std::string& units)
    : timestamp(std::chrono::system_clock::now())
    , label(label)
    , value(value)
    , units(units) {
}

DataCollector::DataCollector(const std::string& output_file, OutputFormat format)
    : output_file_(output_file)
    , format_(format)
    , header_written_(false) {
    
    // Open the file for writing
    file_stream_.open(output_file_, std::ios::out | std::ios::trunc);
    if (!file_stream_.is_open()) {
        throw std::runtime_error("Failed to open output file: " + output_file_);
    }
    
    // Write header based on format
    WriteHeader();
}

DataCollector::~DataCollector() {
    try {
        // Write footer and close file
        if (file_stream_.is_open()) {
            WriteFooter();
            file_stream_.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in DataCollector destructor: " << e.what() << std::endl;
    }
}

bool DataCollector::CollectDataPoint(const std::string& label, double value, const std::string& units) {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Create data point
        DataPoint data_point(label, value, units);
        
        // Add to collection
        data_points_.push_back(data_point);
        
        // Write to file
        return WriteDataPoint(data_point);
    } catch (const std::exception& e) {
        std::cerr << "Error collecting data point: " << e.what() << std::endl;
        return false;
    }
}

bool DataCollector::CollectData(const std::string& data_point) {
    // Legacy method - parse the string and delegate to the new method
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Create a simple data point with the raw string
        DataPoint point("raw_data", 0.0, "");
        point.label = data_point;
        
        // Add to collection
        data_points_.push_back(point);
        
        // Write to file
        return WriteDataPoint(point);
    } catch (const std::exception& e) {
        std::cerr << "Error collecting data: " << e.what() << std::endl;
        return false;
    }
}

bool DataCollector::Flush() {
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Flush the file stream
        file_stream_.flush();
        
        return !file_stream_.fail();
    } catch (const std::exception& e) {
        std::cerr << "Error flushing data: " << e.what() << std::endl;
        return false;
    }
}

size_t DataCollector::GetDataPointCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return data_points_.size();
}

bool DataCollector::WriteHeader() {
    if (header_written_) {
        return true;
    }
    
    try {
        switch (format_) {
            case OutputFormat::CSV:
                file_stream_ << "Timestamp,Label,Value,Units\n";
                break;
                
            case OutputFormat::JSON:
                file_stream_ << "{\n  \"data_points\": [\n";
                break;
                
            case OutputFormat::PLAINTEXT:
                file_stream_ << "=== NVMe-oF Benchmark Data ===\n\n";
                file_stream_ << std::left << std::setw(25) << "Timestamp" 
                           << std::setw(30) << "Label" 
                           << std::setw(15) << "Value" 
                           << "Units\n";
                file_stream_ << std::string(80, '-') << "\n";
                break;
        }
        
        header_written_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing header: " << e.what() << std::endl;
        return false;
    }
}

bool DataCollector::WriteDataPoint(const DataPoint& data_point) {
    if (!header_written_) {
        if (!WriteHeader()) {
            return false;
        }
    }
    
    try {
        // Format timestamp
        auto time_t = std::chrono::system_clock::to_time_t(data_point.timestamp);
        std::stringstream timestamp_ss;
        timestamp_ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        
        switch (format_) {
            case OutputFormat::CSV:
                file_stream_ << timestamp_ss.str() << ","
                           << data_point.label << ","
                           << data_point.value << ","
                           << data_point.units << "\n";
                break;
                
            case OutputFormat::JSON: {
                // Check if this isn't the first data point (need a comma)
                if (data_points_.size() > 1) {
                    file_stream_ << ",\n";
                }
                
                file_stream_ << "    {\n"
                           << "      \"timestamp\": \"" << timestamp_ss.str() << "\",\n"
                           << "      \"label\": \"" << data_point.label << "\",\n"
                           << "      \"value\": " << data_point.value << ",\n"
                           << "      \"units\": \"" << data_point.units << "\"\n"
                           << "    }";
                break;
            }
                
            case OutputFormat::PLAINTEXT:
                file_stream_ << std::left << std::setw(25) << timestamp_ss.str()
                           << std::setw(30) << data_point.label
                           << std::setw(15) << data_point.value
                           << data_point.units << "\n";
                break;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing data point: " << e.what() << std::endl;
        return false;
    }
}

bool DataCollector::WriteFooter() {
    try {
        switch (format_) {
            case OutputFormat::CSV:
                // CSV doesn't need a footer
                break;
                
            case OutputFormat::JSON:
                file_stream_ << "\n  ]\n}\n";
                break;
                
            case OutputFormat::PLAINTEXT:
                file_stream_ << std::string(80, '-') << "\n";
                file_stream_ << "Total data points: " << data_points_.size() << "\n";
                break;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing footer: " << e.what() << std::endl;
        return false;
    }
}

}  // namespace benchmarking
}  // namespace nvmeof