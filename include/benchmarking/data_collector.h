#pragma once

#include <string>

namespace nvmeof {
namespace benchmarking {

class DataCollector {
public:
    explicit DataCollector(const std::string& output_file);

    void CollectData(const std::string& data_point);

private:
    std::string output_file_;
};

}  // namespace benchmarking
}  // namespace nvmeof