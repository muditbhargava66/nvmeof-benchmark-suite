#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <getopt.h>

#include "../include/benchmarking/workload_generator.h"
#include "../include/benchmarking/data_collector.h"
#include "../include/benchmarking/result_visualizer.h"
#include "../include/bottleneck_analysis/system_profiler.h"
#include "../include/bottleneck_analysis/resource_monitor.h"
#include "../include/bottleneck_analysis/bottleneck_detector.h"
#include "../include/optimization_engine/config_knowledge_base.h"
#include "../include/optimization_engine/optimizer.h"
#include "../include/optimization_engine/config_applicator.h"
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
    std::string workload_profile;
    std::string output_dir;
    std::string config_file;
    bool verbose;
    bool optimize;
    bool visualize;
    bool monitor_resources;
    int monitor_interval_ms;
};

// Print usage information
void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n";
    std::cout << "NVMe-oF Benchmarking Suite\n\n";
    std::cout << "Options:\n";
    std::cout << "  -w, --workload-profile FILE   Specify the workload profile JSON file\n";
    std::cout << "  -o, --output-dir DIR          Specify the output directory for results\n";
    std::cout << "  -c, --config-file FILE        Specify the configuration file\n";
    std::cout << "  -v, --verbose                 Enable verbose output\n";
    std::cout << "  -O, --optimize                Enable automatic optimization\n";
    std::cout << "  -V, --visualize               Visualize results after benchmark\n";
    std::cout << "  -m, --monitor                 Enable resource monitoring\n";
    std::cout << "  -i, --interval MS             Monitoring interval in milliseconds (default: 1000)\n";
    std::cout << "  -h, --help                    Display this help message\n";
}

// Parse command-line arguments
bool parseCommandLine(int argc, char** argv, CommandLineOptions& options) {
    static struct option long_options[] = {
        {"workload-profile", required_argument, 0, 'w'},
        {"output-dir",       required_argument, 0, 'o'},
        {"config-file",      required_argument, 0, 'c'},
        {"verbose",          no_argument,       0, 'v'},
        {"optimize",         no_argument,       0, 'O'},
        {"visualize",        no_argument,       0, 'V'},
        {"monitor",          no_argument,       0, 'm'},
        {"interval",         required_argument, 0, 'i'},
        {"help",             no_argument,       0, 'h'},
        {0,                  0,                 0,  0 }
    };

    // Set default values
    options.verbose = false;
    options.optimize = false;
    options.visualize = false;
    options.monitor_resources = false;
    options.monitor_interval_ms = 1000;

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "w:o:c:vOVmi:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'w':
                options.workload_profile = optarg;
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
            case 'O':
                options.optimize = true;
                break;
            case 'V':
                options.visualize = true;
                break;
            case 'm':
                options.monitor_resources = true;
                break;
            case 'i':
                options.monitor_interval_ms = std::stoi(optarg);
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
    if (options.workload_profile.empty()) {
        std::cerr << "Error: Workload profile must be specified\n";
        printUsage(argv[0]);
        return false;
    }

    if (options.output_dir.empty()) {
        // Use default output directory
        options.output_dir = "./data/benchmark_results";
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

// Load workload profile from JSON file
nvmeof::benchmarking::WorkloadProfile loadWorkloadProfile(const std::string& filename) {
    std::string json_content = nvmeof::utils::ReadFileToString(filename);
    if (json_content.empty()) {
        throw std::runtime_error("Failed to read workload profile file: " + filename);
    }

    // In a real implementation, this would parse the JSON
    // For the example, we'll create a dummy profile
    nvmeof::benchmarking::WorkloadProfile profile;
    profile.total_size = 1048576;    // 1 MB
    profile.block_size = 4096;       // 4 KB
    profile.num_blocks = 256;        // 256 blocks
    profile.interval_us = 100;       // 100 microseconds
    profile.read_percentage = 70;    // 70% reads
    profile.write_percentage = 30;   // 30% writes
    profile.random_percentage = 50;  // 50% random

    return profile;
}

int main(int argc, char** argv) {
    // Set up signal handling
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Print banner
    std::cout << "=======================================" << std::endl;
    std::cout << "NVMe-oF Benchmarking Suite" << std::endl;
    std::cout << "=======================================" << std::endl;

    // Parse command-line arguments
    CommandLineOptions options;
    if (!parseCommandLine(argc, argv, options)) {
        return EXIT_FAILURE;
    }

    // Print system information
    if (options.verbose) {
        std::cout << "System Information:" << std::endl;
        std::cout << "  OS:       " << nvmeof::utils::HardwareDetection::GetOSName() 
                 << " " << nvmeof::utils::HardwareDetection::GetOSVersion() << std::endl;
        std::cout << "  CPU:      " << nvmeof::utils::HardwareDetection::GetCPUModel() << std::endl;
        std::cout << "  Cores:    " << nvmeof::utils::HardwareDetection::GetCPUCores() << std::endl;
        std::cout << "  Sockets:  " << nvmeof::utils::HardwareDetection::GetCPUSockets() << std::endl;
        
        std::cout << "  NVMe Devices:" << std::endl;
        auto nvme_devices = nvmeof::utils::HardwareDetection::GetNVMeDevices();
        if (nvme_devices.empty()) {
            std::cout << "    No NVMe devices found" << std::endl;
        } else {
            for (const auto& device : nvme_devices) {
                std::cout << "    - " << device << std::endl;
            }
        }
        std::cout << std::endl;
    }

    try {
        // Load workload profile
        std::cout << "Loading workload profile: " << options.workload_profile << std::endl;
        
        // Comment out or remove the unused workload_profile variable
        // auto workload_profile = loadWorkloadProfile(options.workload_profile);
        
        // For a real implementation, you would use the workload profile here
        // For example: auto generator = createWorkloadGenerator(workload_profile);
        // generator.run();

        // Set up output file path
        std::string timestamp = nvmeof::utils::GetCurrentTimestamp("%Y%m%d_%H%M%S");
        std::string output_file = options.output_dir + "/benchmark_" + timestamp + ".csv";
        
        // Create data collector
        std::cout << "Creating data collector, output file: " << output_file << std::endl;
        nvmeof::benchmarking::DataCollector collector(output_file);

        // Set up resource monitoring if enabled
        std::unique_ptr<nvmeof::bottleneck_analysis::ResourceMonitor> resource_monitor;
        if (options.monitor_resources) {
            std::cout << "Starting resource monitoring with interval: " 
                     << options.monitor_interval_ms << "ms" << std::endl;
            
            resource_monitor = std::make_unique<nvmeof::bottleneck_analysis::ResourceMonitor>(
                std::chrono::milliseconds(options.monitor_interval_ms),
                [&collector](const nvmeof::bottleneck_analysis::ResourceUsage& usage) {
                    // Log CPU usage
                    collector.CollectDataPoint("CPU Usage", usage.cpu_usage_percent, "%");
                    
                    // Log memory usage
                    double memory_usage_percent = usage.GetMemoryUsagePercent();
                    collector.CollectDataPoint("Memory Usage", memory_usage_percent, "%");
                    
                    // Log network usage for each interface
                    for (size_t i = 0; i < usage.interfaces.size(); ++i) {
                        std::string label = "Network RX: " + usage.interfaces[i];
                        collector.CollectDataPoint(label, usage.rx_bytes[i], "bytes");
                        
                        label = "Network TX: " + usage.interfaces[i];
                        collector.CollectDataPoint(label, usage.tx_bytes[i], "bytes");
                    }
                }
            );
            
            resource_monitor->Start();
        }

        // Set up bottleneck detection if optimization is enabled
        std::unique_ptr<nvmeof::bottleneck_analysis::BottleneckDetector> bottleneck_detector;
        std::unique_ptr<nvmeof::optimization_engine::ConfigKnowledgeBase> config_kb;
        std::unique_ptr<nvmeof::optimization_engine::Optimizer> optimizer;
        
        if (options.optimize) {
            std::cout << "Setting up bottleneck detection and optimization" << std::endl;
            
            // Create bottleneck detector with default thresholds
            bottleneck_detector = std::make_unique<nvmeof::bottleneck_analysis::BottleneckDetector>(
                80.0, 90.0, 1000000000, 500000000
            );
            
            // Load optimization knowledge base
            if (!options.config_file.empty()) {
                config_kb = std::make_unique<nvmeof::optimization_engine::ConfigKnowledgeBase>(
                    options.config_file
                );
                
                // Create optimizer
                optimizer = std::make_unique<nvmeof::optimization_engine::Optimizer>(
                    *config_kb, *bottleneck_detector
                );
            } else {
                std::cout << "Warning: No configuration file specified, optimization disabled" << std::endl;
            }
        }

        // Generate and run the workload
        std::cout << "Starting benchmark with profile: " << options.workload_profile << std::endl;
        
        // In a real implementation, this would create and run the workload generator
        // For the example, we'll simulate the benchmark
        collector.CollectDataPoint("Benchmark Start", 0, "");
        
        int progress = 0;
        while (g_running && progress < 100) {
            // Simulate workload execution
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Update progress
            progress += 5;
            if (progress > 100) progress = 100;
            
            // Log progress
            collector.CollectDataPoint("Progress", progress, "%");
            
            // Generate some simulated metrics
            double throughput = 1000.0 + (std::rand() % 500);
            double iops = 250000.0 + (std::rand() % 50000);
            double latency = 100.0 + (std::rand() % 50);
            
            collector.CollectDataPoint("Throughput", throughput, "MB/s");
            collector.CollectDataPoint("IOPS", iops, "ops/s");
            collector.CollectDataPoint("Latency", latency, "µs");
            
            // If optimization is enabled, periodically check for bottlenecks
            if (options.optimize && optimizer && resource_monitor) {
                auto usage = resource_monitor->GetLatestUsage();
                
                // Calculate network usage (sum of all interfaces)
                uint64_t network_rx_total = 0;
                uint64_t network_tx_total = 0;
                for (size_t i = 0; i < usage.interfaces.size(); ++i) {
                    network_rx_total += usage.rx_bytes[i];
                    network_tx_total += usage.tx_bytes[i];
                }
                
                // Optimize configuration based on resource usage
                optimizer->OptimizeConfiguration(
                    usage.cpu_usage_percent,
                    usage.GetMemoryUsagePercent(),
                    network_rx_total + network_tx_total
                );
            }
            
            // Print progress
            if (options.verbose) {
                std::cout << "Progress: " << progress << "%" 
                         << ", Throughput: " << throughput << " MB/s"
                         << ", IOPS: " << iops << " ops/s"
                         << ", Latency: " << latency << " µs"
                         << std::endl;
            }
        }
        
        collector.CollectDataPoint("Benchmark End", 0, "");
        
        // Stop resource monitoring if it was started
        if (resource_monitor) {
            std::cout << "Stopping resource monitoring" << std::endl;
            resource_monitor->Stop();
        }
        
        // Visualize results if requested
        if (options.visualize) {
            std::cout << "Visualizing benchmark results" << std::endl;
            nvmeof::benchmarking::ResultVisualizer visualizer(output_file);
            visualizer.Visualize();
        }
        
        std::cout << "Benchmark completed. Results saved to: " << output_file << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    std::cout << "=======================================" << std::endl;
    return EXIT_SUCCESS;
}