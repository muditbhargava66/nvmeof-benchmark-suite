#include "../../include/utils/nvmeof_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <stdexcept>
#include <mutex>
#include <chrono>
#include <random>
#include <cstdlib> // for getenv, setenv

namespace fs = std::filesystem;

namespace nvmeof {
namespace utils {

// Static mutex for thread-safe operations
static std::mutex utils_mutex;

std::vector<std::string> SplitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    
    // Handle empty string case
    if (str.empty()) {
        tokens.push_back("");
        return tokens;
    }
    
    std::istringstream iss(str);
    std::string token;
    
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    // Check if the last character is the delimiter, in which case add an empty token
    if (!str.empty() && str.back() == delimiter) {
        tokens.push_back("");
    }
    
    return tokens;
}

std::string TrimString(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), [](char c) {
        return std::isspace(static_cast<unsigned char>(c));
    });
    
    auto end = std::find_if_not(str.rbegin(), str.rend(), [](char c) {
        return std::isspace(static_cast<unsigned char>(c));
    }).base();
    
    return (start < end) ? std::string(start, end) : std::string();
}

bool FileExists(const std::string& filename) {
    std::error_code ec;
    return fs::exists(filename, ec) && !ec;
}

bool DirectoryExists(const std::string& dirname) {
    std::error_code ec;
    return fs::exists(dirname, ec) && fs::is_directory(dirname, ec) && !ec;
}

bool CreateDirectory(const std::string& dirname) {
    std::error_code ec;
    
    // If directory already exists, return true
    if (fs::exists(dirname, ec) && fs::is_directory(dirname, ec)) {
        return true;
    }
    
    return fs::create_directories(dirname, ec) || !ec;
}

bool RemoveFile(const std::string& filename) {
    std::error_code ec;
    return fs::remove(filename, ec) && !ec;
}

std::string ReadFileToString(const std::string& filename) {
    if (!FileExists(filename)) {
        std::cerr << "Error: File does not exist: " << filename << std::endl;
        return "";
    }
    
    std::lock_guard<std::mutex> lock(utils_mutex);
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }
    
    try {
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        return buffer.str();
    } catch (const std::exception& e) {
        std::cerr << "Error reading file: " << e.what() << std::endl;
        file.close();
        return "";
    }
}

bool WriteStringToFile(const std::string& filename, const std::string& content) {
    std::lock_guard<std::mutex> lock(utils_mutex);
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }
    
    try {
        file << content;
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing to file: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool AppendStringToFile(const std::string& filename, const std::string& content) {
    std::lock_guard<std::mutex> lock(utils_mutex);
    std::ofstream file(filename, std::ios::out | std::ios::app | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for appending: " << filename << std::endl;
        return false;
    }
    
    try {
        file << content;
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error appending to file: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

std::string FormatTimestamp(const std::chrono::system_clock::time_point& time_point, const std::string& format) {
    std::time_t time_t = std::chrono::system_clock::to_time_t(time_point);
    struct tm tm_buf;
    #ifdef _WIN32
        localtime_s(&tm_buf, &time_t);
    #else
        localtime_r(&time_t, &tm_buf);
    #endif
    
    std::stringstream ss;
    ss << std::put_time(&tm_buf, format.c_str());
    return ss.str();
}

std::string GetCurrentTimestamp(const std::string& format) {
    return FormatTimestamp(std::chrono::system_clock::now(), format);
}

std::string FormatByteSize(uint64_t bytes) {
    const char* suffix[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB"};
    const int suffix_count = sizeof(suffix) / sizeof(suffix[0]);
    
    int i = 0;
    double dblBytes = static_cast<double>(bytes);
    
    if (bytes > 0) {
        for (i = 0; i < suffix_count - 1 && dblBytes >= 1024.0; i++) {
            dblBytes /= 1024.0;
        }
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << dblBytes << " " << suffix[i];
    return ss.str();
}

uint32_t GenerateRandomNumber(uint32_t min, uint32_t max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(min, max);
    return dist(gen);
}

std::string GenerateUniqueId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());
    
    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    // Combine timestamp and random number
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << now_ms;
    ss << std::hex << std::setfill('0') << std::setw(16) << dist(gen);
    
    return ss.str();
}

bool ValidateIpAddress(const std::string& ip_address) {
    // Simple IPv4 address validation using regex would be better,
    // but for simplicity, we'll do a basic check
    size_t dot_count = std::count(ip_address.begin(), ip_address.end(), '.');
    if (dot_count != 3) {
        return false;
    }
    
    auto octets = SplitString(ip_address, '.');
    if (octets.size() != 4) {
        return false;
    }
    
    for (const auto& octet : octets) {
        if (octet.empty() || octet.size() > 3) {
            return false;
        }
        
        for (char c : octet) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                return false;
            }
        }
        
        int octet_value = std::stoi(octet);
        if (octet_value < 0 || octet_value > 255) {
            return false;
        }
    }
    
    return true;
}

std::string ReadEnvironmentVariable(const std::string& var_name, const std::string& default_value) {
    const char* value = std::getenv(var_name.c_str());
    return value ? value : default_value;
}

void SetEnvironmentVariable(const std::string& var_name, const std::string& value) {
#ifdef _WIN32
    _putenv_s(var_name.c_str(), value.c_str());
#else
    setenv(var_name.c_str(), value.c_str(), 1);
#endif
}

bool ParseBooleanString(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), 
                  [](unsigned char c) { return std::tolower(c); });
    
    return (lower_str == "true" || lower_str == "yes" || lower_str == "1" || lower_str == "on");
}

}  // namespace utils
}  // namespace nvmeof