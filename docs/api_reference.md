# API Reference

This document provides a reference for the public APIs and interfaces exposed by the NVMe-oF Benchmarking Suite. It serves as a guide for developers who want to integrate the benchmarking suite into their own applications or extend its functionality.

## Workload Generator

### `WorkloadGenerator` Class

The `WorkloadGenerator` class is responsible for generating realistic workloads based on configurable profiles.

#### Constructor

```cpp
WorkloadGenerator(const std::string& profile_path);
```

- `profile_path`: The path to the workload profile JSON file.

#### Member Functions

```cpp
void Generate();
```

Generates the workload based on the specified profile.

```cpp
const std::vector<Workload>& GetWorkloads() const;
```

Returns the generated workloads.

## Benchmarking Engine

### `BenchmarkingEngine` Class

The `BenchmarkingEngine` class executes the workloads and collects performance metrics.

#### Constructor

```cpp
BenchmarkingEngine(const std::vector<Workload>& workloads);
```

- `workloads`: The workloads to be executed.

#### Member Functions

```cpp
void Run();
```

Executes the benchmarking process.

```cpp
const BenchmarkResults& GetResults() const;
```

Returns the benchmark results.

## Bottleneck Analyzer

### `BottleneckAnalyzer` Class

The `BottleneckAnalyzer` class analyzes the benchmark results to identify performance bottlenecks.

#### Constructor

```cpp
BottleneckAnalyzer(const BenchmarkResults& results);
```

- `results`: The benchmark results to analyze.

#### Member Functions

```cpp
void Analyze();
```

Performs the bottleneck analysis.

```cpp
const std::vector<Bottleneck>& GetBottlenecks() const;
```

Returns the identified bottlenecks.

## Optimization Engine

### `OptimizationEngine` Class

The `OptimizationEngine` class provides recommendations and automated tools to optimize NVMe-oF configurations.

#### Constructor

```cpp
OptimizationEngine(const std::vector<Bottleneck>& bottlenecks);
```

- `bottlenecks`: The identified bottlenecks.

#### Member Functions

```cpp
void Optimize();
```

Performs the optimization process.

```cpp
const OptimizationResults& GetResults() const;
```

Returns the optimization results.

## Visualization and Reporting

### `Visualizer` Class

The `Visualizer` class presents the benchmark results and analysis in a user-friendly format.

#### Constructor

```cpp
Visualizer(const BenchmarkResults& benchmark_results, const OptimizationResults& optimization_results);
```

- `benchmark_results`: The benchmark results.
- `optimization_results`: The optimization results.

#### Member Functions

```cpp
void GenerateReport(const std::string& output_path);
```

Generates a visual report of the benchmark results and optimization recommendations.

- `output_path`: The path where the report will be generated.

## Utility Functions

### `LoadWorkloadProfile`

```cpp
WorkloadProfile LoadWorkloadProfile(const std::string& profile_path);
```

Loads a workload profile from a JSON file.

- `profile_path`: The path to the workload profile JSON file.
- Returns: The loaded workload profile.

### `SaveBenchmarkResults`

```cpp
void SaveBenchmarkResults(const BenchmarkResults& results, const std::string& output_path);
```

Saves the benchmark results to a file.

- `results`: The benchmark results to save.
- `output_path`: The path where the results will be saved.

### `LoadBenchmarkResults`

```cpp
BenchmarkResults LoadBenchmarkResults(const std::string& input_path);
```

Loads the benchmark results from a file.

- `input_path`: The path to the file containing the benchmark results.
- Returns: The loaded benchmark results.

## Conclusion

This API Reference provides an overview of the main classes and functions exposed by the NVMe-oF Benchmarking Suite. Developers can use these APIs to integrate the benchmarking functionality into their own applications or extend the suite's capabilities.

For more detailed information about each API and its usage, please refer to the source code documentation and the accompanying user guide and developer guide.

If you have any questions or need further assistance, please don't hesitate to reach out to the project maintainers or the developer community.

Happy coding and happy benchmarking!

---