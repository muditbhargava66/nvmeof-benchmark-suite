#include "utils/nvmeof_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

namespace nvmeof {
namespace utils {

std::vector<std::string> SplitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string TrimString(const std::string& str) {
    std::string trimmed = str;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1);
    return trimmed;
}

bool FileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

std::string ReadFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

void WriteStringToFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }
    file << content;
    file.close();
}

}  // namespace utils
}  // namespace nvmeof