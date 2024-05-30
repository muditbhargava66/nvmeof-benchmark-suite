# NVMe-oF Benchmarking Suite

The NVMe-oF Benchmarking Suite is a comprehensive toolkit for evaluating the performance of NVMe-over-Fabrics (NVMe-oF) storage systems. It provides a set of tools and utilities to generate realistic workloads, measure performance metrics, analyze bottlenecks, and optimize configurations for NVMe-oF deployments.

## Features

- Workload Generation: Generate realistic workloads based on configurable profiles to simulate various application scenarios.
- Performance Measurement: Collect and record performance metrics such as latency, throughput, and IOPS during workload execution.
- Bottleneck Analysis: Identify performance bottlenecks in the NVMe-oF stack, including hardware, software, and configuration aspects.
- Optimization Engine: Provide recommendations and automated tools to tune NVMe-oF parameters for optimal performance based on workload characteristics and hardware configuration.
- Visualization and Reporting: Present benchmark results and analysis in a user-friendly format through charts, graphs, and detailed reports.

## Getting Started

### Prerequisites

- Linux operating system (tested on Ubuntu 20.04 LTS)
- GCC compiler (version 9.3.0 or higher)
- CMake (version 3.10 or higher)
- NVMe-oF target and initiator setup

### Installation

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

### Usage

1. Configure the workload profiles in the `data/workload_profiles` directory. Each profile is defined in a JSON file and specifies the characteristics of the workload, such as total size, block size, number of blocks, and interval.

2. Run the benchmarks using the `run_benchmarks.sh` script:
   ```
   ./scripts/run_benchmarks.sh
   ```
   The script will execute the benchmarks for each workload profile and store the results in the `data/benchmark_results` directory.

3. Analyze the benchmark results using the `analyze_results.sh` script:
   ```
   ./scripts/analyze_results.sh
   ```
   The script will process the benchmark results, identify bottlenecks, and generate analysis reports in the `data/analysis_reports` directory.

4. Visualize the benchmark results and analysis using the provided visualization tools or by manually inspecting the generated reports.

## Directory Structure

| Directory                 | Usage                                                                      |
|---------------------------|----------------------------------------------------------------------------|
| `src/`                    | Contains the source code files for the project.                            |
| `include/`                | Contains the header files for the project.                                 |
| `tests/`                  | Contains the unit tests and integration tests for the project.             |
| `data/`                   | Contains data files used by the project, such as workload profiles.        |
| `scripts/`                | Contains utility scripts for building, running benchmarks, and analysis.   |
| `third_party/`            | Contains third-party libraries used by the project.                        |
| `docs/`                   | Contains project documentation.                                            |

## Future Enhancements

- Support for additional NVMe-oF transport protocols, such as TCP and RoCE.
- Integration with popular storage benchmarking tools like FIO and IOzone.
- Automatic generation of optimal NVMe-oF configurations based on workload characteristics and hardware profiles.
- Enhanced visualization and reporting capabilities, including interactive dashboards and customizable reports.

## Contributing

Contributions to the NVMe-oF Benchmarking Suite are welcome! If you find any issues or have suggestions for improvements, please open an issue or submit a pull request on the project's GitHub repository.

When contributing, please adhere to the existing code style, use meaningful commit messages, and provide appropriate documentation for new features or changes.

## License

This project is licensed under the [MIT License](LICENSE).

## Contact

For any questions or inquiries, please contact the project maintainer at [muditbhargava66](https://github.com/muditbhargava66).

---