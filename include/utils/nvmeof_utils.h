#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>

namespace nvmeof {
namespace utils {

/**
 * @brief Splits a string into tokens using the specified delimiter.
 * 
 * @param str The string to split
 * @param delimiter The character to use as delimiter
 * 
 * @return A vector of tokens
 */
std::vector<std::string> SplitString(const std::string& str, char delimiter);

/**
 * @brief Removes leading and trailing whitespace from a string.
 * 
 * @param str The string to trim
 * 
 * @return The trimmed string
 */
std::string TrimString(const std::string& str);

/**
 * @brief Checks if a file exists.
 * 
 * @param filename The path to the file
 * 
 * @return true if the file exists, false otherwise
 */
bool FileExists(const std::string& filename);

/**
 * @brief Checks if a directory exists.
 * 
 * @param dirname The path to the directory
 * 
 * @return true if the directory exists, false otherwise
 */
bool DirectoryExists(const std::string& dirname);

/**
 * @brief Creates a directory (and any parent directories) if it doesn't exist.
 * 
 * @param dirname The path of the directory to create
 * 
 * @return true if the directory was created or already exists, false otherwise
 */
bool CreateDirectory(const std::string& dirname);

/**
 * @brief Removes a file.
 * 
 * @param filename The path to the file to remove
 * 
 * @return true if the file was removed, false otherwise
 */
bool RemoveFile(const std::string& filename);

/**
 * @brief Reads a file into a string.
 * 
 * @param filename The path to the file
 * 
 * @return The contents of the file as a string, or an empty string on error
 */
std::string ReadFileToString(const std::string& filename);

/**
 * @brief Writes a string to a file, overwriting any existing content.
 * 
 * @param filename The path to the file
 * @param content The content to write
 * 
 * @return true if the write was successful, false otherwise
 */
bool WriteStringToFile(const std::string& filename, const std::string& content);

/**
 * @brief Appends a string to a file.
 * 
 * @param filename The path to the file
 * @param content The content to append
 * 
 * @return true if the append was successful, false otherwise
 */
bool AppendStringToFile(const std::string& filename, const std::string& content);

/**
 * @brief Formats a timestamp as a string.
 * 
 * @param time_point The time point to format
 * @param format The format string (default: "%Y-%m-%d %H:%M:%S")
 * 
 * @return The formatted timestamp
 */
std::string FormatTimestamp(
    const std::chrono::system_clock::time_point& time_point,
    const std::string& format = "%Y-%m-%d %H:%M:%S"
);

/**
 * @brief Gets the current timestamp as a formatted string.
 * 
 * @param format The format string (default: "%Y-%m-%d %H:%M:%S")
 * 
 * @return The formatted current timestamp
 */
std::string GetCurrentTimestamp(const std::string& format = "%Y-%m-%d %H:%M:%S");

/**
 * @brief Formats a byte size in a human-readable format (B, KB, MB, GB, etc.).
 * 
 * @param bytes The size in bytes
 * 
 * @return The formatted size (e.g., "1.25 MB")
 */
std::string FormatByteSize(uint64_t bytes);

/**
 * @brief Generates a random number in the specified range.
 * 
 * @param min The minimum value (inclusive)
 * @param max The maximum value (inclusive)
 * 
 * @return A random number in the range [min, max]
 */
uint32_t GenerateRandomNumber(uint32_t min, uint32_t max);

/**
 * @brief Generates a unique ID based on timestamp and random number.
 * 
 * @return A unique ID string
 */
std::string GenerateUniqueId();

/**
 * @brief Validates an IPv4 address string.
 * 
 * @param ip_address The IPv4 address to validate
 * 
 * @return true if the address is valid, false otherwise
 */
bool ValidateIpAddress(const std::string& ip_address);

/**
 * @brief Gets the value of an environment variable.
 * 
 * @param var_name The name of the environment variable
 * @param default_value The default value to return if the variable is not set
 * 
 * @return The value of the environment variable or the default value
 */
std::string ReadEnvironmentVariable(const std::string& var_name, const std::string& default_value = "");

/**
 * @brief Sets the value of an environment variable.
 * 
 * @param var_name The name of the environment variable
 * @param value The value to set
 */
void SetEnvironmentVariable(const std::string& var_name, const std::string& value);

/**
 * @brief Parses a string as a boolean value.
 * 
 * Recognizes "true", "yes", "1", "on" as true (case-insensitive).
 * 
 * @param str The string to parse
 * 
 * @return The boolean value
 */
bool ParseBooleanString(const std::string& str);

}  // namespace utils
}  // namespace nvmeof