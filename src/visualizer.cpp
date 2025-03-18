#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>
#include <chrono>
#include <thread>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <algorithm>
#include <numeric>
#include <iomanip>

#include "../include/benchmarking/data_collector.h"
#include "../include/benchmarking/result_visualizer.h"
#include "../include/bottleneck_analysis/bottleneck_detector.h"
#include "../include/utils/nvmeof_utils.h"

// Global flag for signal handling
volatile sig_atomic_t g_running = 1;

// Signal handler
void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down gracefully..." << std::endl;
    g_running = 0;
}

// Command-line options
struct CommandLineOptions {
    std::string input_file;
    std::string output_file;
    bool verbose;
    bool export_chart;
    std::string chart_type;
    std::string metrics;
    int terminal_width;
    int terminal_height;
};

// Print usage information
void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n";
    std::cout << "NVMe-oF Benchmarking Suite - Visualization Tool\n\n";
    std::cout << "Options:\n";
    std::cout << "  -i, --input-file FILE      Specify the benchmark results file to visualize\n";
    std::cout << "  -o, --output-file FILE     Specify the output file for exported visualization\n";
    std::cout << "  -v, --verbose              Enable verbose output\n";
    std::cout << "  -e, --export               Export visualization to a file\n";
    std::cout << "  -c, --chart-type TYPE      Specify chart type (line, bar, heatmap, histogram)\n";
    std::cout << "  -m, --metrics LIST         Comma-separated list of metrics to visualize\n";
    std::cout << "  -w, --width WIDTH          Terminal width for visualization (default: auto)\n";
    std::cout << "  -h, --height HEIGHT        Terminal height for visualization (default: auto)\n";
    std::cout << "  --help                     Display this help message\n";
}

// Parse command-line arguments
bool parseCommandLine(int argc, char** argv, CommandLineOptions& options) {
    static struct option long_options[] = {
        {"input-file",  required_argument, 0, 'i'},
        {"output-file", required_argument, 0, 'o'},
        {"verbose",     no_argument,       0, 'v'},
        {"export",      no_argument,       0, 'e'},
        {"chart-type",  required_argument, 0, 'c'},
        {"metrics",     required_argument, 0, 'm'},
        {"width",       required_argument, 0, 'w'},
        {"height",      required_argument, 0, 'H'},
        {"help",        no_argument,       0, 'h'},
        {0,             0,                 0,  0 }
    };

    // Set default values
    options.verbose = false;
    options.export_chart = false;
    options.chart_type = "line";
    options.terminal_width = 0;  // Auto
    options.terminal_height = 0; // Auto

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "i:o:vec:m:w:H:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'i':
                options.input_file = optarg;
                break;
            case 'o':
                options.output_file = optarg;
                break;
            case 'v':
                options.verbose = true;
                break;
            case 'e':
                options.export_chart = true;
                break;
            case 'c':
                options.chart_type = optarg;
                // Validate chart type
                if (options.chart_type != "line" && 
                    options.chart_type != "bar" && 
                    options.chart_type != "heatmap" && 
                    options.chart_type != "histogram") {
                    std::cerr << "Error: Invalid chart type. Valid types are: line, bar, heatmap, histogram\n";
                    return false;
                }
                break;
            case 'm':
                options.metrics = optarg;
                break;
            case 'w':
                try {
                    options.terminal_width = std::stoi(optarg);
                    if (options.terminal_width < 10) {
                        std::cerr << "Error: Terminal width must be at least 10\n";
                        return false;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error: Invalid terminal width\n";
                    return false;
                }
                break;
            case 'H':
                try {
                    options.terminal_height = std::stoi(optarg);
                    if (options.terminal_height < 5) {
                        std::cerr << "Error: Terminal height must be at least 5\n";
                        return false;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error: Invalid terminal height\n";
                    return false;
                }
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
    if (options.input_file.empty()) {
        std::cerr << "Error: Input file must be specified\n";
        printUsage(argv[0]);
        return false;
    }

    if (!nvmeof::utils::FileExists(options.input_file)) {
        std::cerr << "Error: Input file does not exist: " << options.input_file << std::endl;
        return false;
    }

    // If export is requested, output file is required
    if (options.export_chart && options.output_file.empty()) {
        std::cerr << "Error: Output file must be specified when exporting\n";
        return false;
    }

    return true;
}

// Data structure to hold parsed benchmark data
struct BenchmarkData {
    std::vector<std::string> timestamps;
    std::map<std::string, std::vector<double>> metrics;
    std::map<std::string, std::string> units;
};

// Parse benchmark data from a CSV file
BenchmarkData parseBenchmarkData(const std::string& filename) {
    BenchmarkData data;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file: " << filename << std::endl;
        return data;
    }
    
    std::string line;
    
    // Read header
    std::getline(file, line);
    std::istringstream header_iss(line);
    std::string header_cell;
    std::vector<std::string> headers;
    
    // Parse CSV header
    while (std::getline(header_iss, header_cell, ',')) {
        headers.push_back(header_cell);
    }
    
    // Simple validation
    if (headers.size() < 4 || headers[0] != "Timestamp" || headers[1] != "Label" || 
        headers[2] != "Value" || headers[3] != "Units") {
        std::cerr << "Error: Invalid CSV format. Expected headers: Timestamp,Label,Value,Units" << std::endl;
        return data;
    }
    
    // Process data rows
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string timestamp, label, value_str, units;
        
        if (std::getline(iss, timestamp, ',') && 
            std::getline(iss, label, ',') && 
            std::getline(iss, value_str, ',') && 
            std::getline(iss, units)) {
            
            try {
                // Skip non-metric entries like "Benchmark Start" or "Benchmark End"
                if (label == "Benchmark Start" || label == "Benchmark End") {
                    continue;
                }
                
                double value = std::stod(value_str);
                
                // Store the timestamp if it's new
                if (std::find(data.timestamps.begin(), data.timestamps.end(), timestamp) == data.timestamps.end()) {
                    data.timestamps.push_back(timestamp);
                }
                
                // Store the metric value
                data.metrics[label].push_back(value);
                
                // Store the unit for this metric if not already stored
                if (data.units.find(label) == data.units.end()) {
                    data.units[label] = units;
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Warning: Failed to parse value: " << value_str << std::endl;
            }
        }
    }
    
    return data;
}

// Generate a simple ASCII line chart for terminal display
void drawAsciiLineChart(const std::string& title, 
                     const std::vector<double>& values,
                     const std::vector<std::string>& timestamps,
                     const std::string& units,
                     int width = 80, 
                     int height = 15) {
    // Mark height parameter as used to suppress warning
    (void)height;
    if (values.empty()) {
        std::cout << "No data to visualize." << std::endl;
        return;
    }
    
    // Calculate chart dimensions
    int chart_width = width - 10;  // Leave space for y-axis labels
    int chart_height = height - 4; // Leave space for title and x-axis
    
    // Find min and max values
    auto [min_it, max_it] = std::minmax_element(values.begin(), values.end());
    double min_value = *min_it;
    double max_value = *max_it;
    
    // Adjust min/max to create a nice range
    double range = max_value - min_value;
    
    // If all values are the same, create a small range around it
    if (range == 0) {
        min_value = min_value * 0.9;
        max_value = max_value * 1.1;
        range = max_value - min_value;
    }
    
    // Apply some padding to the range
    min_value -= range * 0.05;
    max_value += range * 0.05;
    range = max_value - min_value;
    
    // Print chart title
    std::cout << std::endl;
    std::cout << title << " (" << units << ")" << std::endl;
    
    // Draw the chart
    std::vector<std::string> chart(chart_height, std::string(chart_width, ' '));
    
    // Generate y-axis labels
    int num_labels = 5;
    std::vector<double> y_labels;
    for (int i = 0; i < num_labels; ++i) {
        double value = min_value + (range * (num_labels - 1 - i)) / (num_labels - 1);
        y_labels.push_back(value);
    }
    
    // Draw y-axis labels
    for (int i = 0; i < num_labels; ++i) {
        int row = i * (chart_height - 1) / (num_labels - 1);
        std::cout << std::setw(8) << std::fixed << std::setprecision(1) << y_labels[i] << " │";
        
        // Draw horizontal grid lines
        for (int x = 0; x < chart_width; ++x) {
            chart[row][x] = '.';
        }
    }
    
    // Plot the data points
    int num_points = values.size();
    for (int i = 0; i < num_points && i < chart_width; ++i) {
        int x = i * chart_width / (num_points - 1);
        double normalized_value = (values[i] - min_value) / range;
        int y = std::round((1.0 - normalized_value) * (chart_height - 1));
        
        // Ensure y is within bounds
        y = std::max(0, std::min(chart_height - 1, y));
        
        chart[y][x] = '*';
    }
    
    // Draw the chart
    for (int y = 0; y < chart_height; ++y) {
        if (y > 0 && y < num_labels) {
            std::cout << "         ";
        } else {
            std::cout << "        │";
        }
        std::cout << chart[y] << std::endl;
    }
    
    // Draw x-axis
    std::cout << "        └";
    for (int x = 0; x < chart_width; ++x) {
        std::cout << "─";
    }
    std::cout << std::endl;
    
    // Draw x-axis labels (timestamps)
    if (!timestamps.empty()) {
        std::cout << "         ";
        
        // Show first, middle, and last timestamps
        int first_idx = 0;
        int middle_idx = timestamps.size() / 2;
        int last_idx = timestamps.size() - 1;
        
        // Format abbreviation of timestamps
        auto format_timestamp = [](const std::string& ts) -> std::string {
            // Extract only the time part HH:MM:SS
            size_t pos = ts.find(' ');
            if (pos != std::string::npos && pos + 1 < ts.size()) {
                return ts.substr(pos + 1);
            }
            return ts;
        };
        
        // Draw timestamp labels
        for (int i = 0; i < chart_width; ++i) {
            if (i == 0) {
                std::cout << format_timestamp(timestamps[first_idx]);
            } else if (i == chart_width / 2) {
                std::cout << format_timestamp(timestamps[middle_idx]);
            } else if (i == chart_width - 1) {
                std::cout << format_timestamp(timestamps[last_idx]);
            } else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
    
    // Print summary statistics
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    double avg = sum / values.size();
    
    std::cout << std::endl;
    std::cout << "Summary: ";
    std::cout << "Min=" << *min_it << ", ";
    std::cout << "Max=" << *max_it << ", ";
    std::cout << "Avg=" << avg << " " << units << std::endl;
    std::cout << std::endl;
}

// Generate a simple ASCII bar chart for terminal display
void drawAsciiBarChart(const std::string& title, 
                     const std::map<std::string, double>& values,
                     const std::string& units,
                     int width = 80, 
                     int height = 15) {
    // Mark height parameter as used to suppress warning
    (void)height;
    if (values.empty()) {
        std::cout << "No data to visualize." << std::endl;
        return;
    }
    
    // Calculate chart dimensions
    int chart_width = width - 20;  // Leave space for labels
    
    // Find max value
    double max_value = 0.0;
    for (const auto& [label, value] : values) {
        max_value = std::max(max_value, value);
    }
    
    // Add some padding to max value
    max_value *= 1.1;
    
    // Print chart title
    std::cout << std::endl;
    std::cout << title << " (" << units << ")" << std::endl;
    std::cout << std::endl;
    
    // Calculate label width
    int max_label_width = 0;
    for (const auto& [label, value] : values) {
        max_label_width = std::max(max_label_width, static_cast<int>(label.length()));
    }
    max_label_width = std::min(max_label_width, 15); // Limit to 15 characters
    
    // Draw the bars
    for (const auto& [label, value] : values) {
        // Format the label to fixed width
        std::string display_label = label.substr(0, max_label_width);
        std::cout << std::setw(max_label_width) << std::left << display_label << " │ ";
        
        // Draw the bar
        int bar_length = static_cast<int>((value / max_value) * chart_width);
        for (int i = 0; i < bar_length; ++i) {
            std::cout << "█";
        }
        
        // Print the value at the end of the bar
        std::cout << " " << std::fixed << std::setprecision(2) << value << " " << units << std::endl;
    }
    
    std::cout << std::endl;
}

// Generate a simple ASCII histogram for terminal display
void drawAsciiHistogram(const std::string& title, 
                      const std::vector<double>& values,
                      const std::string& units,
                      int width = 80, 
                      int height = 15,
                      int num_bins = 10) {
    // Mark height parameter as used to suppress warning
    (void)height;
    if (values.empty()) {
        std::cout << "No data to visualize." << std::endl;
        return;
    }
    
    // Calculate chart dimensions
    int chart_width = width - 20;  // Leave space for labels
    
    // Find min and max values
    auto [min_it, max_it] = std::minmax_element(values.begin(), values.end());
    double min_value = *min_it;
    double max_value = *max_it;
    
    // Calculate bin width
    double range = max_value - min_value;
    double bin_width = range / num_bins;
    
    // Create bins
    std::vector<int> bins(num_bins, 0);
    for (double value : values) {
        int bin_index = std::min(num_bins - 1, static_cast<int>((value - min_value) / bin_width));
        bins[bin_index]++;
    }
    
    // Find max bin count for scaling
    int max_bin_count = *std::max_element(bins.begin(), bins.end());
    
    // Print chart title
    std::cout << std::endl;
    std::cout << title << " Histogram (" << units << ")" << std::endl;
    std::cout << std::endl;
    
    // Draw the histogram
    for (int i = 0; i < num_bins; ++i) {
        // Calculate bin range
        double bin_start = min_value + i * bin_width;
        double bin_end = bin_start + bin_width;
        
        // Format the bin label
        std::string bin_label = std::to_string(static_cast<int>(bin_start)) + "-" + 
                                std::to_string(static_cast<int>(bin_end));
        
        std::cout << std::setw(10) << std::left << bin_label << " │ ";
        
        // Draw the bar
        int bar_length = static_cast<int>(static_cast<double>(bins[i]) / max_bin_count * chart_width);
        for (int j = 0; j < bar_length; ++j) {
            std::cout << "█";
        }
        
        // Print the count at the end of the bar
        std::cout << " " << bins[i] << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "Total samples: " << values.size() << std::endl;
    std::cout << "Min: " << min_value << " " << units << ", Max: " << max_value << " " << units << std::endl;
    std::cout << std::endl;
}

// Generate a simple ASCII heatmap for terminal display
void drawAsciiHeatmap(const std::string& title,
                    const std::map<std::string, std::vector<double>>& metrics,
                    const std::map<std::string, std::string>& units,
                    int width = 80,
                    int height = 20) {
    // Mark height parameter as used to suppress warning
    (void)height;
    if (metrics.empty()) {
        std::cout << "No data to visualize." << std::endl;
        return;
    }
    
    // Get the first metric to determine data size
    const auto& first_metric = metrics.begin()->second;
    if (first_metric.empty()) {
        std::cout << "No data points to visualize." << std::endl;
        return;
    }
    
    // Find global min and max for color scaling
    double global_min = std::numeric_limits<double>::max();
    double global_max = std::numeric_limits<double>::lowest();
    
    for (const auto& [metric_name, metric_values] : metrics) {
        if (!metric_values.empty()) {
            auto [min_it, max_it] = std::minmax_element(metric_values.begin(), metric_values.end());
            global_min = std::min(global_min, *min_it);
            global_max = std::max(global_max, *max_it);
        }
    }
    
    // Add padding to min/max
    double range = global_max - global_min;
    if (range == 0) {
        // If all values are the same, create a small range
        global_min = global_min * 0.9;
        global_max = global_max * 1.1;
    }
    
    // Print chart title
    std::cout << std::endl;
    std::cout << title << " Heatmap" << std::endl;
    std::cout << std::endl;
    
    // Calculate label width
    int max_label_width = 0;
    for (const auto& [metric_name, _] : metrics) {
        max_label_width = std::max(max_label_width, static_cast<int>(metric_name.length()));
    }
    max_label_width = std::min(max_label_width, 15); // Limit to 15 characters
    
    // ANSI color codes for gradient (blue to red)
    const std::vector<std::string> colors = {
        "\033[38;5;17m", // Dark blue
        "\033[38;5;27m", // Blue
        "\033[38;5;39m", // Light blue
        "\033[38;5;51m", // Cyan
        "\033[38;5;48m", // Teal
        "\033[38;5;46m", // Green
        "\033[38;5;226m", // Yellow
        "\033[38;5;208m", // Orange
        "\033[38;5;196m"  // Red
    };
    
    // Reset color code
    const std::string reset_color = "\033[0m";
    
    // Function to get color based on value
    auto get_color = [&](double value) -> std::string {
        double normalized = (value - global_min) / (global_max - global_min);
        int color_index = std::min(static_cast<int>(normalized * colors.size()), static_cast<int>(colors.size() - 1));
        return colors[color_index];
    };
    
    // Function to get character based on value
    auto get_char = [&](double value) -> char {
        double normalized = (value - global_min) / (global_max - global_min);
        if (normalized < 0.2) return ' ';
        if (normalized < 0.4) return '.';
        if (normalized < 0.6) return '#';
        if (normalized < 0.8) return '@';
        return '*';
    };
    
    // Draw column headers (time points)
    int num_time_points = first_metric.size();
    int max_display_points = width - max_label_width - 3;
    int step = std::max(1, num_time_points / max_display_points);
    
    std::cout << std::setw(max_label_width + 3) << " ";
    for (int i = 0; i < num_time_points; i += step) {
        std::cout << "T" << i;
        if (i < num_time_points - step) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    
    // Draw the heatmap
    for (const auto& [metric_name, metric_values] : metrics) {
        // Format the label to fixed width
        std::string display_label = metric_name.substr(0, max_label_width);
        std::cout << std::setw(max_label_width) << std::left << display_label << " │ ";
        
        // Get the unit for this metric
        std::string unit = units.count(metric_name) ? units.at(metric_name) : "";
        
        // Draw the heatmap cells
        for (size_t i = 0; i < metric_values.size(); i += step) {
            double value = metric_values[i];
            std::string color = get_color(value);
            char cell_char = get_char(value);
            
            std::cout << color << cell_char << cell_char << reset_color;
        }
        
        // Print the unit at the end
        std::cout << " " << unit << std::endl;
    }
    
    // Print legend
    std::cout << std::endl << "Legend: ";
    int num_legend_steps = 5;
    for (int i = 0; i < num_legend_steps; ++i) {
        double value = global_min + (global_max - global_min) * i / (num_legend_steps - 1);
        std::string color = get_color(value);
        char cell_char = get_char(value);
        
        std::cout << color << cell_char << cell_char << reset_color << " " 
                  << std::fixed << std::setprecision(1) << value << " ";
    }
    std::cout << std::endl << std::endl;
}

// Visualize benchmark data using various chart types
void visualizeBenchmarkData(const BenchmarkData& data, const CommandLineOptions& options) {
    if (data.metrics.empty() || data.timestamps.empty()) {
        std::cerr << "Error: No valid data to visualize." << std::endl;
        return;
    }
    
    // Determine terminal dimensions if not specified
    int term_width = options.terminal_width;
    int term_height = options.terminal_height;
    
    if (term_width <= 0 || term_height <= 0) {
        // Try to get terminal dimensions using ioctl or fallback to defaults
        term_width = 80;
        term_height = 20;
        
        #ifdef TIOCGSIZE
            struct ttysize ts;
            if (ioctl(STDIN_FILENO, TIOCGSIZE, &ts) >= 0) {
                term_width = ts.ts_cols;
                term_height = ts.ts_lines;
            }
        #elif defined(TIOCGWINSZ)
            struct winsize ws;
            if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) >= 0) {
                term_width = ws.ws_col;
                term_height = ws.ws_row;
            }
        #endif
    }
    
    // Filter metrics based on user request
    std::vector<std::string> selected_metrics;
    if (!options.metrics.empty()) {
        std::istringstream iss(options.metrics);
        std::string metric;
        while (std::getline(iss, metric, ',')) {
            metric = nvmeof::utils::TrimString(metric);
            if (!metric.empty() && data.metrics.find(metric) != data.metrics.end()) {
                selected_metrics.push_back(metric);
            }
        }
    } else {
        // Default: select all metrics except progress
        for (const auto& [metric_name, _] : data.metrics) {
            if (metric_name != "Progress") {
                selected_metrics.push_back(metric_name);
            }
        }
    }
    
    if (selected_metrics.empty()) {
        std::cerr << "Error: No valid metrics selected for visualization." << std::endl;
        return;
    }
    
    std::cout << "=======================================" << std::endl;
    std::cout << "NVMe-oF Benchmarking Suite - Visualization" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "Source: " << options.input_file << std::endl;
    std::cout << "Metrics: " << selected_metrics.size() << " selected" << std::endl;
    std::cout << "Data points: " << data.timestamps.size() << std::endl;
    std::cout << "Chart type: " << options.chart_type << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    
    if (options.chart_type == "line") {
        // Draw a line chart for each selected metric
        for (const auto& metric_name : selected_metrics) {
            const auto& values = data.metrics.at(metric_name);
            const auto& unit = data.units.at(metric_name);
            
            drawAsciiLineChart(metric_name, values, data.timestamps, unit, term_width, term_height);
        }
    } else if (options.chart_type == "bar") {
        // For bar chart, we'll use the average value of each metric
        std::map<std::string, double> avg_values;
        for (const auto& metric_name : selected_metrics) {
            const auto& values = data.metrics.at(metric_name);
            if (!values.empty()) {
                double sum = std::accumulate(values.begin(), values.end(), 0.0);
                avg_values[metric_name] = sum / values.size();
            }
        }
        
        // Draw bar chart with average values and the unit of the first metric
        std::string unit = data.units.at(selected_metrics[0]);
        drawAsciiBarChart("Average Metric Values", avg_values, unit, term_width, term_height);
    } else if (options.chart_type == "histogram") {
        // Draw a histogram for each selected metric
        for (const auto& metric_name : selected_metrics) {
            const auto& values = data.metrics.at(metric_name);
            const auto& unit = data.units.at(metric_name);
            
            drawAsciiHistogram(metric_name, values, unit, term_width, term_height);
        }
    } else if (options.chart_type == "heatmap") {
        // Create a subset of metrics for the heatmap
        std::map<std::string, std::vector<double>> selected_metric_data;
        std::map<std::string, std::string> selected_metric_units;
        
        for (const auto& metric_name : selected_metrics) {
            selected_metric_data[metric_name] = data.metrics.at(metric_name);
            selected_metric_units[metric_name] = data.units.at(metric_name);
        }
        
        // Draw the heatmap
        drawAsciiHeatmap("Performance Metrics", selected_metric_data, selected_metric_units, term_width, term_height);
    }
    
    // Print summary statistics
    std::cout << "Summary Statistics:" << std::endl;
    std::cout << std::left << std::setw(20) << "Metric" 
              << std::setw(10) << "Min" 
              << std::setw(10) << "Max" 
              << std::setw(10) << "Avg" 
              << "Unit" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    for (const auto& metric_name : selected_metrics) {
        const auto& values = data.metrics.at(metric_name);
        const auto& unit = data.units.at(metric_name);
        
        if (!values.empty()) {
            double min_val = *std::min_element(values.begin(), values.end());
            double max_val = *std::max_element(values.begin(), values.end());
            double sum = std::accumulate(values.begin(), values.end(), 0.0);
            double avg_val = sum / values.size();
            
            std::cout << std::left << std::setw(20) << metric_name 
                      << std::setw(10) << std::fixed << std::setprecision(2) << min_val 
                      << std::setw(10) << max_val 
                      << std::setw(10) << avg_val 
                      << unit << std::endl;
        }
    }
    
    std::cout << std::endl;
}

// Export visualization to a file (simplified version - in practice, this would generate actual charts)
void exportVisualization(const BenchmarkData& data, const CommandLineOptions& options) {
    if (options.output_file.empty()) {
        std::cerr << "Error: No output file specified for export." << std::endl;
        return;
    }
    
    std::ofstream output(options.output_file);
    if (!output.is_open()) {
        std::cerr << "Error: Unable to create output file: " << options.output_file << std::endl;
        return;
    }
    
    // Write a basic HTML report with embedded charts
    output << "<!DOCTYPE html>\n";
    output << "<html>\n";
    output << "<head>\n";
    output << "    <title>NVMe-oF Benchmark Visualization</title>\n";
    output << "    <style>\n";
    output << "        body { font-family: Arial, sans-serif; margin: 20px; }\n";
    output << "        h1, h2 { color: #333; }\n";
    output << "        table { border-collapse: collapse; width: 100%; margin-bottom: 20px; }\n";
    output << "        th, td { text-align: left; padding: 8px; border-bottom: 1px solid #ddd; }\n";
    output << "        th { background-color: #f2f2f2; }\n";
    output << "        .chart { margin: 20px 0; padding: 10px; background-color: #f9f9f9; border-radius: 5px; }\n";
    output << "    </style>\n";
    output << "</head>\n";
    output << "<body>\n";
    output << "    <h1>NVMe-oF Benchmark Visualization</h1>\n";
    output << "    <p>Source file: " << options.input_file << "</p>\n";
    output << "    <p>Generated on: " << nvmeof::utils::GetCurrentTimestamp() << "</p>\n";
    output << "    <p>Data points: " << data.timestamps.size() << "</p>\n";
    
    // Summary statistics table
    output << "    <h2>Summary Statistics</h2>\n";
    output << "    <table>\n";
    output << "        <tr>\n";
    output << "            <th>Metric</th>\n";
    output << "            <th>Min</th>\n";
    output << "            <th>Max</th>\n";
    output << "            <th>Avg</th>\n";
    output << "            <th>Unit</th>\n";
    output << "        </tr>\n";
    
    for (const auto& [metric_name, values] : data.metrics) {
        // Skip "Progress" metric
        if (metric_name == "Progress") {
            continue;
        }
        
        if (!values.empty()) {
            double min_val = *std::min_element(values.begin(), values.end());
            double max_val = *std::max_element(values.begin(), values.end());
            double sum = std::accumulate(values.begin(), values.end(), 0.0);
            double avg_val = sum / values.size();
            const auto& unit = data.units.at(metric_name);
            
            output << "        <tr>\n";
            output << "            <td>" << metric_name << "</td>\n";
            output << "            <td>" << std::fixed << std::setprecision(2) << min_val << "</td>\n";
            output << "            <td>" << std::fixed << std::setprecision(2) << max_val << "</td>\n";
            output << "            <td>" << std::fixed << std::setprecision(2) << avg_val << "</td>\n";
            output << "            <td>" << unit << "</td>\n";
            output << "        </tr>\n";
        }
    }
    
    output << "    </table>\n";
    
    // Placeholder for charts (in a real implementation, this would generate actual charts)
    output << "    <h2>Performance Charts</h2>\n";
    output << "    <p>Note: This is a placeholder. In a production version, this file would contain actual interactive charts.</p>\n";
    
    for (const auto& [metric_name, values] : data.metrics) {
        // Skip "Progress" metric
        if (metric_name == "Progress") {
            continue;
        }
        
        if (!values.empty()) {
            const auto& unit = data.units.at(metric_name);
            
            output << "    <div class=\"chart\">\n";
            output << "        <h3>" << metric_name << " (" << unit << ")</h3>\n";
            output << "        <div>[Chart placeholder for " << metric_name << "]</div>\n";
            output << "    </div>\n";
        }
    }
    
    output << "</body>\n";
    output << "</html>\n";
    
    output.close();
    
    std::cout << "Visualization exported to: " << options.output_file << std::endl;
}

int main(int argc, char** argv) {
    // Set up signal handling
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Parse command-line arguments
    CommandLineOptions options;
    if (!parseCommandLine(argc, argv, options)) {
        return EXIT_FAILURE;
    }
    
    try {
        // Parse the benchmark data
        std::cout << "Parsing benchmark data from: " << options.input_file << std::endl;
        BenchmarkData data = parseBenchmarkData(options.input_file);
        
        if (data.metrics.empty() || data.timestamps.empty()) {
            std::cerr << "Error: No valid data found in the input file." << std::endl;
            return EXIT_FAILURE;
        }
        
        if (options.verbose) {
            std::cout << "Found " << data.metrics.size() << " metrics and " 
                      << data.timestamps.size() << " data points." << std::endl;
            
            for (const auto& [metric_name, values] : data.metrics) {
                std::cout << "  - " << metric_name << ": " 
                          << values.size() << " values, Unit: " 
                          << data.units.at(metric_name) << std::endl;
            }
        }
        
        // Visualize the data in the terminal
        visualizeBenchmarkData(data, options);
        
        // Export visualization if requested
        if (options.export_chart) {
            exportVisualization(data, options);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}