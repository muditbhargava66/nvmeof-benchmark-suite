#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <chrono>
#include <numeric>
#include <map>
#include <unordered_map>
#include <thread>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <getopt.h>

#include "../include/benchmarking/data_collector.h"
#include "../include/benchmarking/result_visualizer.h"
#include "../include/bottleneck_analysis/bottleneck_detector.h"
#include "../include/bottleneck_analysis/system_profiler.h"
#include "../include/optimization_engine/config_knowledge_base.h"
#include "../include/optimization_engine/optimizer.h"
#include "../include/utils/nvmeof_utils.h"
#include "../include/utils/hardware_detection.h"

// Global flag for signal handling
volatile sig_atomic_t g_running = 1;

// Signal handler
void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down gracefully..." << std::endl;
    g_running = 0;
}

// Command-line options
struct CommandLineOptions {
    std::string results_file;
    std::string output_dir;
    std::string config_file;
    bool verbose;
    bool generate_report;
    bool recommend_optimizations;
};

// Print usage information
void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n";
    std::cout << "NVMe-oF Benchmarking Suite - Analysis Tool\n\n";
    std::cout << "Options:\n";
    std::cout << "  -r, --results-file FILE     Specify the benchmark results file to analyze\n";
    std::cout << "  -d, --results-dir DIR       Specify the directory containing benchmark results\n";
    std::cout << "  -o, --output-dir DIR        Specify the output directory for analysis reports\n";
    std::cout << "  -c, --config-file FILE      Specify the optimization configuration file\n";
    std::cout << "  -v, --verbose               Enable verbose output\n";
    std::cout << "  -g, --generate-report       Generate a detailed analysis report\n";
    std::cout << "  -p, --recommend             Recommend performance optimizations\n";
    std::cout << "  -h, --help                  Display this help message\n";
}

// Parse command-line arguments
bool parseCommandLine(int argc, char** argv, CommandLineOptions& options) {
    static struct option long_options[] = {
        {"results-file",    required_argument, 0, 'r'},
        {"results-dir",     required_argument, 0, 'd'},
        {"output-dir",      required_argument, 0, 'o'},
        {"config-file",     required_argument, 0, 'c'},
        {"verbose",         no_argument,       0, 'v'},
        {"generate-report", no_argument,       0, 'g'},
        {"recommend",       no_argument,       0, 'p'},
        {"help",            no_argument,       0, 'h'},
        {0,                 0,                 0,  0 }
    };

    // Set default values
    options.verbose = false;
    options.generate_report = false;
    options.recommend_optimizations = false;

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "r:d:o:c:vgph", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'r':
                options.results_file = optarg;
                break;
            case 'd':
                // Will be used if no specific results file is provided
                // to analyze all files in the directory
                if (options.results_file.empty()) {
                    std::filesystem::path dir_path(optarg);
                    // Find the most recent benchmark file
                    if (std::filesystem::exists(dir_path) && std::filesystem::is_directory(dir_path)) {
                        std::filesystem::file_time_type latest_time;
                        std::filesystem::path latest_file;
                        
                        for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
                            if (entry.is_regular_file() && entry.path().extension() == ".csv" &&
                                entry.path().filename().string().find("benchmark_") != std::string::npos) {
                                auto file_time = std::filesystem::last_write_time(entry.path());
                                if (latest_file.empty() || file_time > latest_time) {
                                    latest_time = file_time;
                                    latest_file = entry.path();
                                }
                            }
                        }
                        
                        if (!latest_file.empty()) {
                            options.results_file = latest_file.string();
                            if (options.verbose) {
                                std::cout << "Found latest benchmark file: " << options.results_file << std::endl;
                            }
                        }
                    }
                }
                break;
            case 'o':
                options.output_dir = optarg;
                break;
            case 'c':
                options.config_file = optarg;
                break;
            case 'v':
                options.verbose = true;
                break;
            case 'g':
                options.generate_report = true;
                break;
            case 'p':
                options.recommend_optimizations = true;
                break;
            case 'h':
                printUsage(argv[0]);
                exit(EXIT_SUCCESS);
            case '?':
                // getopt_long already printed an error message
                return false;
            default:
                return false;
        }
    }

    // Validate required options
    if (options.results_file.empty()) {
        std::cerr << "Error: Results file must be specified with -r or directory with -d\n";
        printUsage(argv[0]);
        return false;
    }

    if (!nvmeof::utils::FileExists(options.results_file)) {
        std::cerr << "Error: Results file does not exist: " << options.results_file << std::endl;
        return false;
    }

    if (options.output_dir.empty()) {
        // Use default output directory
        options.output_dir = "./data/analysis_reports";
    }

    // Ensure output directory exists
    if (!nvmeof::utils::DirectoryExists(options.output_dir)) {
        std::cout << "Creating output directory: " << options.output_dir << std::endl;
        if (!nvmeof::utils::CreateDirectory(options.output_dir)) {
            std::cerr << "Error: Failed to create output directory\n";
            return false;
        }
    }

    return true;
}

// Parse a CSV file containing benchmark results
std::vector<std::pair<std::string, double>> parseBenchmarkResults(const std::string& filename) {
    std::vector<std::pair<std::string, double>> results;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file: " << filename << std::endl;
        return results;
    }
    
    std::string line;
    // Skip header line
    std::getline(file, line);
    
    // Process data lines
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string timestamp, label, value_str, units;
        
        if (std::getline(iss, timestamp, ',') && 
            std::getline(iss, label, ',') && 
            std::getline(iss, value_str, ',') && 
            std::getline(iss, units)) {
            
            try {
                double value = std::stod(value_str);
                results.emplace_back(label, value);
            } catch (const std::exception& e) {
                if (value_str != "0" && !value_str.empty()) {
                    std::cerr << "Warning: Failed to parse value: " << value_str << std::endl;
                }
            }
        }
    }
    
    return results;
}

// Calculate metrics from benchmark results
void calculateMetrics(const std::vector<std::pair<std::string, double>>& results, 
                      std::map<std::string, std::vector<double>>& metrics,
                      std::map<std::string, std::pair<double, double>>& metric_summary) {
    
    // Group data by metric type
    for (const auto& result : results) {
        // Skip progress or non-numeric metrics
        if (result.first == "Progress" || result.first == "Benchmark Start" || result.first == "Benchmark End") {
            continue;
        }
        
        metrics[result.first].push_back(result.second);
    }
    
    // Calculate summary statistics (min, max, avg)
    for (const auto& [metric, values] : metrics) {
        if (values.empty()) {
            continue;
        }
        
        double sum = 0.0;
        double min_val = values[0];
        double max_val = values[0];
        
        for (const auto& value : values) {
            sum += value;
            min_val = std::min(min_val, value);
            max_val = std::max(max_val, value);
        }
        
        double avg_val = sum / values.size();
        // Mark avg_val as used to suppress warning
        (void)avg_val;
        metric_summary[metric] = {min_val, max_val};
    }
}

// Generate a detailed analysis report
void generateReport(const std::string& output_path, 
                   const std::vector<std::pair<std::string, double>>& results,
                   const std::map<std::string, std::vector<double>>& metrics,
                   const std::map<std::string, std::pair<double, double>>& metric_summary,
                   const std::vector<nvmeof::bottleneck_analysis::BottleneckInfo>& bottlenecks) {
    
    std::ofstream report(output_path);
    if (!report.is_open()) {
        std::cerr << "Error: Unable to create report file: " << output_path << std::endl;
        return;
    }
    
    // Write report header
    report << "# NVMe-oF Benchmarking Suite - Analysis Report\n";
    report << "Date: " << nvmeof::utils::GetCurrentTimestamp() << "\n\n";
    
    // Write summary section
    report << "## Performance Summary\n\n";
    
    for (const auto& [metric, stats] : metric_summary) {
        double min_val = stats.first;
        double max_val = stats.second;
        
        // Calculate average from the metrics vector
        const auto& values = metrics.at(metric);
        double sum = std::accumulate(values.begin(), values.end(), 0.0);
        double avg_val = sum / values.size();
        
        // Determine units (parsed from the results)
        std::string units;
        for (const auto& result : results) {
            if (result.first == metric) {
                // Find if the metric ends with a common unit string
                for (const auto& unit : {"MB/s", "KB/s", "ops/s", "µs", "ms", "%"}) {
                    if (result.first.find(unit) != std::string::npos) {
                        units = unit;
                        break;
                    }
                }
                break;
            }
        }
        
        report << "### " << metric << "\n\n";
        report << "- Minimum: " << min_val << " " << units << "\n";
        report << "- Maximum: " << max_val << " " << units << "\n";
        report << "- Average: " << avg_val << " " << units << "\n\n";
    }
    
    // Write bottleneck analysis section
    report << "## Bottleneck Analysis\n\n";
    
    if (bottlenecks.empty()) {
        report << "No significant bottlenecks detected.\n\n";
    } else {
        for (const auto& bottleneck : bottlenecks) {
            report << "### " << bottleneck.resource_name << " Bottleneck\n\n";
            report << "- Severity: " << (bottleneck.severity * 100.0) << "%\n";
            report << "- Description: " << bottleneck.description << "\n";
            report << "- Resource Usage: " << bottleneck.resource_usage;
            
            // Add units based on resource type
            if (bottleneck.type == nvmeof::bottleneck_analysis::BottleneckType::CPU || 
                bottleneck.type == nvmeof::bottleneck_analysis::BottleneckType::MEMORY) {
                report << "%";
            } else if (bottleneck.type == nvmeof::bottleneck_analysis::BottleneckType::NETWORK || 
                       bottleneck.type == nvmeof::bottleneck_analysis::BottleneckType::STORAGE) {
                report << " bytes/s";
            }
            
            report << "\n";
            report << "- Recommendation: " << bottleneck.recommendation << "\n\n";
        }
    }
    
    // Write system information section
    report << "## System Information\n\n";
    report << "- OS: " << nvmeof::bottleneck_analysis::SystemProfiler::GetOSInfo() << "\n";
    report << "- CPU: " << nvmeof::bottleneck_analysis::SystemProfiler::GetCPUInfo() << "\n";
    report << "- Total Memory: " << nvmeof::utils::FormatByteSize(nvmeof::bottleneck_analysis::SystemProfiler::GetTotalMemory()) << "\n";
    
    report << "- Network Interfaces:\n";
    auto interfaces = nvmeof::bottleneck_analysis::SystemProfiler::GetNetworkInterfaces();
    for (const auto& interface : interfaces) {
        report << "  - " << interface << "\n";
    }
    
    report << "\n## Raw Metrics\n\n";
    report << "The analysis was based on " << results.size() << " data points.\n";
    
    report.close();
    
    std::cout << "Analysis report generated: " << output_path << std::endl;
}

// Analyze benchmark results for bottlenecks
std::vector<nvmeof::bottleneck_analysis::BottleneckInfo> analyzeBottlenecks(
    const std::vector<std::pair<std::string, double>>& results) {
    
    // Extract relevant metrics for bottleneck detection
    double cpu_usage = 0.0;
    double memory_usage = 0.0;
    uint64_t network_usage = 0;
    uint64_t storage_usage = 0;
    
    for (const auto& result : results) {
        if (result.first == "CPU Usage") {
            cpu_usage = std::max(cpu_usage, result.second);
        } else if (result.first == "Memory Usage") {
            memory_usage = std::max(memory_usage, result.second);
        } else if (result.first.find("Network RX") != std::string::npos || 
                   result.first.find("Network TX") != std::string::npos) {
            network_usage += static_cast<uint64_t>(result.second);
        } else if (result.first.find("Disk") != std::string::npos || 
                   result.first.find("Storage") != std::string::npos) {
            storage_usage += static_cast<uint64_t>(result.second);
        }
    }
    
    // Create a detector with default thresholds
    nvmeof::bottleneck_analysis::BottleneckDetector detector(80.0, 90.0, 1000000000, 500000000);
    
    // Detect bottlenecks
    return detector.DetectBottlenecks(cpu_usage, memory_usage, network_usage, storage_usage);
}

// Recommend optimizations based on detected bottlenecks
void recommendOptimizations(
    const std::vector<nvmeof::bottleneck_analysis::BottleneckInfo>& bottlenecks,
    const std::string& config_file,
    const std::string& output_path) {
    
    std::ofstream recommendations(output_path);
    if (!recommendations.is_open()) {
        std::cerr << "Error: Unable to create recommendations file: " << output_path << std::endl;
        return;
    }
    
    // Write header
    recommendations << "# NVMe-oF Performance Optimization Recommendations\n";
    recommendations << "Date: " << nvmeof::utils::GetCurrentTimestamp() << "\n\n";
    
    if (bottlenecks.empty()) {
        recommendations << "No significant bottlenecks detected. No optimizations recommended at this time.\n";
        recommendations.close();
        std::cout << "Optimization recommendations generated: " << output_path << std::endl;
        return;
    }
    
    // Load configuration knowledge base if available
    std::unique_ptr<nvmeof::optimization_engine::ConfigKnowledgeBase> config_kb;
    bool has_config = false;
    
    if (!config_file.empty() && nvmeof::utils::FileExists(config_file)) {
        config_kb = std::make_unique<nvmeof::optimization_engine::ConfigKnowledgeBase>(config_file);
        has_config = true;
    }
    
    // Generate recommendations for each bottleneck
    recommendations << "## Detected Bottlenecks and Recommendations\n\n";
    
    for (const auto& bottleneck : bottlenecks) {
        recommendations << "### " << bottleneck.resource_name << " Bottleneck\n\n";
        recommendations << "- Severity: " << (bottleneck.severity * 100.0) << "%\n";
        recommendations << "- Description: " << bottleneck.description << "\n";
        recommendations << "- Recommendation: " << bottleneck.recommendation << "\n\n";
        
        // Add specific configuration recommendations if available
        if (has_config) {
            std::string config_key;
            switch (bottleneck.type) {
                case nvmeof::bottleneck_analysis::BottleneckType::CPU:
                    config_key = "cpu_bottleneck";
                    break;
                case nvmeof::bottleneck_analysis::BottleneckType::MEMORY:
                    config_key = "memory_bottleneck";
                    break;
                case nvmeof::bottleneck_analysis::BottleneckType::NETWORK:
                    config_key = "network_bottleneck";
                    break;
                case nvmeof::bottleneck_analysis::BottleneckType::STORAGE:
                    config_key = "storage_bottleneck";
                    break;
                default:
                    config_key = "";
                    break;
            }
            
            if (!config_key.empty()) {
                std::string config_value = config_kb->GetConfigValue(config_key);
                if (!config_value.empty()) {
                    recommendations << "#### Recommended Configuration\n\n";
                    recommendations << "```\n" << config_value << "\n```\n\n";
                    
                    // Parse and explain each configuration parameter
                    recommendations << "#### Configuration Explanation\n\n";
                    std::istringstream iss(config_value);
                    std::string param;
                    while (std::getline(iss, param, ',')) {
                        std::string key, value;
                        std::istringstream param_iss(param);
                        if (std::getline(param_iss, key, '=') && std::getline(param_iss, value)) {
                            key = nvmeof::utils::TrimString(key);
                            value = nvmeof::utils::TrimString(value);
                            
                            recommendations << "- `" << key << "=" << value << "`: ";
                            
                            // Provide explanation based on parameter
                            if (key == "cpu_governor") {
                                recommendations << "CPU frequency scaling governor. Setting to 'performance' maintains max CPU frequency for consistent performance.\n";
                            } else if (key == "hugepages") {
                                recommendations << "Number of huge pages to allocate. Huge pages reduce TLB misses and improve memory access performance.\n";
                            } else if (key == "vm.swappiness") {
                                recommendations << "Kernel swappiness parameter. Lower values reduce swap usage, keeping data in RAM.\n";
                            } else if (key == "vm.vfs_cache_pressure") {
                                recommendations << "Controls filesystem cache reclamation. Lower values prioritize keeping directory and inode caches.\n";
                            } else if (key == "net.core.rmem_max" || key == "net.core.wmem_max") {
                                recommendations << "Maximum receive/send socket buffer size. Larger values can improve network throughput.\n";
                            } else if (key == "vm.dirty_ratio" || key == "vm.dirty_background_ratio") {
                                recommendations << "Controls when the kernel starts writing dirty pages to disk. Adjusting these can optimize I/O performance.\n";
                            } else {
                                recommendations << "System parameter that can be tuned for better performance.\n";
                            }
                        }
                    }
                }
            }
        }
        
        // Add additional usage tips based on bottleneck type
        recommendations << "#### Additional Optimization Tips\n\n";
        
        switch (bottleneck.type) {
            case nvmeof::bottleneck_analysis::BottleneckType::CPU:
                recommendations << "- Consider using CPU pinning to dedicate cores to NVMe-oF workloads\n";
                recommendations << "- Adjust interrupt handling using IRQ affinity\n";
                recommendations << "- Try polling mode for lower latency at the cost of higher CPU usage\n";
                recommendations << "- Investigate NUMA placement if your system has multiple sockets\n";
                break;
                
            case nvmeof::bottleneck_analysis::BottleneckType::MEMORY:
                recommendations << "- Increase available system memory if possible\n";
                recommendations << "- Enable and configure huge pages for better memory performance\n";
                recommendations << "- Adjust memory allocation strategies in your application\n";
                recommendations << "- Check for memory leaks or excessive memory usage in applications\n";
                break;
                
            case nvmeof::bottleneck_analysis::BottleneckType::NETWORK:
                recommendations << "- Evaluate network hardware upgrades (NICs, switches, cables)\n";
                recommendations << "- Adjust TCP/IP settings for better network performance\n";
                recommendations << "- Consider using RDMA-capable networks for lower latency\n";
                recommendations << "- Review MTU settings and potentially enable jumbo frames\n";
                break;
                
            case nvmeof::bottleneck_analysis::BottleneckType::STORAGE:
                recommendations << "- Review storage hardware capabilities and consider upgrades\n";
                recommendations << "- Adjust I/O scheduler settings to optimize for your workload\n";
                recommendations << "- Consider using multiple namespaces or devices for parallel I/O\n";
                recommendations << "- Optimize your application's I/O patterns\n";
                break;
                
            default:
                recommendations << "- Review system configuration holistically\n";
                recommendations << "- Monitor performance regularly and adjust settings incrementally\n";
                break;
        }
    }
    
    recommendations.close();
    std::cout << "Optimization recommendations generated: " << output_path << std::endl;
}

int main(int argc, char** argv) {
    // Set up signal handling
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Print banner
    std::cout << "=======================================" << std::endl;
    std::cout << "NVMe-oF Benchmarking Suite - Analysis Tool" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    // Parse command-line arguments
    CommandLineOptions options;
    if (!parseCommandLine(argc, argv, options)) {
        return EXIT_FAILURE;
    }
    
    try {
        // Parse benchmark results
        std::cout << "Analyzing benchmark results from: " << options.results_file << std::endl;
        auto results = parseBenchmarkResults(options.results_file);
        
        if (results.empty()) {
            std::cerr << "Error: No valid data found in the results file." << std::endl;
            return EXIT_FAILURE;
        }
        
        std::cout << "Parsed " << results.size() << " data points." << std::endl;
        
        // Calculate metrics
        std::map<std::string, std::vector<double>> metrics;
        std::map<std::string, std::pair<double, double>> metric_summary;
        calculateMetrics(results, metrics, metric_summary);
        
        // Print summary
        std::cout << "\nPerformance Summary:" << std::endl;
        for (const auto& [metric, stats] : metric_summary) {
            double min_val = stats.first;
            double max_val = stats.second;
            
            // Calculate average
            const auto& values = metrics.at(metric);
            double sum = std::accumulate(values.begin(), values.end(), 0.0);
            double avg_val = sum / values.size();
            
            std::cout << "  " << metric << ": ";
            std::cout << "Min=" << min_val << ", ";
            std::cout << "Max=" << max_val << ", ";
            std::cout << "Avg=" << avg_val << std::endl;
        }
        
        // Detect bottlenecks
        auto bottlenecks = analyzeBottlenecks(results);
        
        std::cout << "\nBottleneck Analysis:" << std::endl;
        if (bottlenecks.empty()) {
            std::cout << "  No significant bottlenecks detected." << std::endl;
        } else {
            for (const auto& bottleneck : bottlenecks) {
                std::cout << "  " << bottleneck.resource_name << " Bottleneck (Severity: " 
                          << (bottleneck.severity * 100.0) << "%)" << std::endl;
                std::cout << "    - " << bottleneck.description << std::endl;
                std::cout << "    - Recommendation: " << bottleneck.recommendation << std::endl;
            }
        }
        
        // Generate detailed report if requested
        if (options.generate_report) {
            std::filesystem::path output_path = options.output_dir;
            std::filesystem::path input_path = options.results_file;
            std::string filename = "analysis_" + 
                                   nvmeof::utils::GetCurrentTimestamp("%Y%m%d_%H%M%S") + 
                                   ".md";
            
            std::filesystem::path report_path = output_path / filename;
            generateReport(report_path.string(), results, metrics, metric_summary, bottlenecks);
        }
        
        // Generate optimization recommendations if requested
        if (options.recommend_optimizations) {
            std::filesystem::path output_path = options.output_dir;
            std::string filename = "recommendations_" + 
                                   nvmeof::utils::GetCurrentTimestamp("%Y%m%d_%H%M%S") + 
                                   ".md";
            
            std::filesystem::path recommendations_path = output_path / filename;
            recommendOptimizations(bottlenecks, options.config_file, recommendations_path.string());
        }
        
        std::cout << "Analysis completed successfully." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during analysis: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    std::cout << "=======================================" << std::endl;
    return EXIT_SUCCESS;
}