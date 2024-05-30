#pragma once

#include <string>
#include <vector>

namespace nvmeof {
namespace utils {

std::vector<std::string> SplitString(const std::string& str, char delimiter);
std::string TrimString(const std::string& str);
bool FileExists(const std::string& filename);
std::string ReadFileToString(const std::string& filename);
void WriteStringToFile(const std::string& filename, const std::string& content);

}  // namespace utils
}  // namespace nvmeof