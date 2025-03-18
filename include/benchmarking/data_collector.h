#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <fstream>
#include <memory>

namespace nvmeof {
namespace benchmarking {

/**
 * @brief Represents a single data point collected during benchmarking.
 */
struct DataPoint {
    std::chrono::system_clock::time_point timestamp; ///< Time when the data point was collected
    std::string label;                              ///< Label describing the data point
    double value;                                   ///< Numeric value of the data point
    std::string units;                              ///< Units of measurement (e.g., "MB/s", "µs")

    /**
     * @brief Creates a new data point with the specified parameters.
     * 
     * @param label Label describing the data point
     * @param value Numeric value of the data point
     * @param units Units of measurement
     */
    DataPoint(const std::string& label, double value, const std::string& units);
};

/**
 * @brief Formats for data collection output.
 */
enum class OutputFormat {
    CSV,      ///< Comma-separated values
    JSON,     ///< JSON format
    PLAINTEXT ///< Plain text format
};

/**
 * @brief Collects and stores benchmark data points.
 * 
 * This class is responsible for collecting data points during benchmark execution
 * and storing them in the specified output file format.
 */
class DataCollector {
public:
    /**
     * @brief Constructs a DataCollector with the specified output file and format.
     * 
     * @param output_file Path to the file where data will be stored
     * @param format Format of the output file (default: CSV)
     * 
     * @throws std::runtime_error If the output file cannot be opened for writing
     */
    explicit DataCollector(const std::string& output_file, OutputFormat format = OutputFormat::CSV);
    
    /**
     * @brief Destroys the DataCollector, flushing any pending data to disk.
     */
    ~DataCollector();

    /**
     * @brief Collects a data point with the specified value and label.
     * 
     * @param label Label describing the data point
     * @param value Numeric value of the data point
     * @param units Units of measurement (e.g., "MB/s", "µs")
     * 
     * @return true if the data point was collected successfully, false otherwise
     */
    bool CollectDataPoint(const std::string& label, double value, const std::string& units);

    /**
     * @brief Collects a data point as a string.
     * 
     * @param data_point String representation of the data point
     * 
     * @return true if the data point was collected successfully, false otherwise
     * 
     * @deprecated Use CollectDataPoint(label, value, units) instead
     */
    [[deprecated("Use CollectDataPoint(label, value, units) instead")]]
    bool CollectData(const std::string& data_point);

    /**
     * @brief Flushes all collected data points to the output file.
     * 
     * @return true if the data was flushed successfully, false otherwise
     */
    bool Flush();

    /**
     * @brief Gets the number of data points collected.
     * 
     * @return The number of data points collected
     */
    size_t GetDataPointCount() const;

private:
    /**
     * @brief Writes the header to the output file based on the selected format.
     * 
     * @return true if the header was written successfully, false otherwise
     */
    bool WriteHeader();

    /**
     * @brief Writes a data point to the output file based on the selected format.
     * 
     * @param data_point The data point to write
     * 
     * @return true if the data point was written successfully, false otherwise
     */
    bool WriteDataPoint(const DataPoint& data_point);

    /**
     * @brief Writes the footer to the output file based on the selected format.
     * 
     * @return true if the footer was written successfully, false otherwise
     */
    bool WriteFooter();

    std::string output_file_;             ///< Path to the output file
    OutputFormat format_;                 ///< Format of the output file
    std::vector<DataPoint> data_points_;  ///< Collected data points
    std::ofstream file_stream_;           ///< Output file stream
    mutable std::mutex mutex_;            ///< Mutex for thread safety
    bool header_written_;                 ///< Flag indicating whether the header has been written
};

}  // namespace benchmarking
}  // namespace nvmeof