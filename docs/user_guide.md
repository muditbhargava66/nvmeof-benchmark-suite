# User Guide

Welcome to the NVMe-oF Benchmarking Suite User Guide! This document will walk you through the process of setting up and using the benchmarking suite to evaluate the performance of NVMe-over-Fabrics (NVMe-oF) storage systems.

## Prerequisites

Before getting started, ensure that you have the following prerequisites:

- Linux operating system (tested on Ubuntu 20.04 LTS)
- GCC compiler (version 9.3.0 or higher)
- CMake (version 3.10 or higher)
- NVMe-oF target and initiator setup

## Installation

Follow these steps to install the NVMe-oF Benchmarking Suite:

1. Clone the repository:
   ```
   git clone https://github.com/muditbhargava66/nvmeof-benchmarking-suite.git
   ```

2. Navigate to the project directory:
   ```
   cd nvmeof-benchmarking-suite
   ```

3. Create a build directory and navigate to it:
   ```
   mkdir build && cd build
   ```

4. Generate the build files using CMake:
   ```
   cmake ..
   ```

5. Build the project:
   ```
   make
   ```

6. (Optional) Install the project:
   ```
   sudo make install
   ```

## Configuration

Before running the benchmarks, you need to configure the workload profiles. The workload profiles define the characteristics of the workloads that will be generated and used for benchmarking.

1. Navigate to the `data/workload_profiles` directory.

2. Create a new JSON file for each workload profile you want to define. For example, `workload_profile_1.json`.

3. Open the JSON file and specify the following parameters:
   - `total_size`: The total size of the workload in bytes.
   - `block_size`: The size of each block in bytes.
   - `num_blocks`: The number of blocks to be generated.
   - `interval_us`: The interval between each block in microseconds.

   Example:
   ```json
   {
     "total_size": 1073741824,
     "block_size": 4096,
     "num_blocks": 262144,
     "interval_us": 100
   }
   ```

4. Save the JSON file.

5. Repeat steps 2-4 for each workload profile you want to define.

## Running Benchmarks

To run the benchmarks, follow these steps:

1. Open a terminal and navigate to the project's root directory.

2. Run the `run_benchmarks.sh` script:
   ```
   ./scripts/run_benchmarks.sh
   ```

   The script will execute the benchmarks for each workload profile defined in the `data/workload_profiles` directory. The benchmark results will be stored in the `data/benchmark_results` directory.

3. Wait for the benchmarks to complete. The duration of the benchmarks will depend on the size and complexity of the workload profiles.

## Analyzing Results

After running the benchmarks, you can analyze the results to identify performance bottlenecks and optimize the NVMe-oF configuration.

1. Open a terminal and navigate to the project's root directory.

2. Run the `analyze_results.sh` script:
   ```
   ./scripts/analyze_results.sh
   ```

   The script will process the benchmark results, generate analysis reports, and store them in the `data/analysis_reports` directory.

3. Open the generated analysis reports to view the performance metrics, identified bottlenecks, and optimization recommendations.

## Visualization

The NVMe-oF Benchmarking Suite provides visualization tools to help you understand and interpret the benchmark results.

1. Open a terminal and navigate to the project's root directory.

2. Run the visualization tool:
   ```
   ./build/bin/nvmeof_visualizer
   ```

   The visualizer will display charts, graphs, and other visual representations of the benchmark results.

3. Use the interactive controls provided by the visualizer to explore different aspects of the benchmark results, such as latency distribution, throughput over time, and IOPS.

## Troubleshooting

If you encounter any issues while using the NVMe-oF Benchmarking Suite, please refer to the following troubleshooting tips:

- Ensure that you have the necessary prerequisites installed and properly configured.
- Double-check the workload profile configurations to ensure they are valid and well-formed JSON files.
- Verify that the NVMe-oF target and initiator are set up correctly and accessible.
- Check the log files generated during the benchmarking process for any error messages or warnings.
- If the issue persists, please open an issue on the project's GitHub repository, providing detailed information about the problem and steps to reproduce it.

## Conclusion

Congratulations! You have now successfully installed, configured, and run the NVMe-oF Benchmarking Suite. By analyzing the benchmark results and utilizing the provided optimization recommendations, you can tune your NVMe-oF storage system for optimal performance.

If you have any further questions or need additional assistance, please refer to the project's documentation or reach out to the project maintainer.

Happy benchmarking!

---