#pragma once

#include <string>

namespace nvmeof {
namespace benchmarking {

class ResultVisualizer {
public:
    explicit ResultVisualizer(const std::string& input_file);

    void Visualize();

private:
    std::string input_file_;
};

}  // namespace benchmarking
}  // namespace nvmeof