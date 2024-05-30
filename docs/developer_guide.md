# Developer Guide

Welcome to the NVMe-oF Benchmarking Suite Developer Guide! This document is intended for developers who want to contribute to the project, extend its functionality, or understand its internal workings.

## Architecture Overview

The NVMe-oF Benchmarking Suite follows a modular architecture, separating concerns into distinct components. The main components of the project are:

- **Workload Generator**: Responsible for generating realistic workloads based on configurable profiles.
- **Benchmarking Engine**: Executes the workloads and collects performance metrics.
- **Bottleneck Analyzer**: Analyzes the benchmark results to identify performance bottlenecks.
- **Optimization Engine**: Provides recommendations and automated tools to optimize NVMe-oF configurations.
- **Visualization and Reporting**: Presents the benchmark results and analysis in a user-friendly format.

## Code Structure

The project's code structure is organized as follows:

- `src/`: Contains the source code files for the project.
  - `workload_generator/`: Implements the workload generation functionality.
  - `benchmarking_engine/`: Implements the benchmarking execution and metric collection.
  - `bottleneck_analyzer/`: Implements the bottleneck analysis logic.
  - `optimization_engine/`: Implements the optimization recommendations and automation.
  - `visualization/`: Implements the visualization and reporting functionality.
  - `utils/`: Contains utility functions and helper classes.

- `include/`: Contains the header files for the project.
  - `workload_generator/`: Contains header files for the workload generator.
  - `benchmarking_engine/`: Contains header files for the benchmarking engine.
  - `bottleneck_analyzer/`: Contains header files for the bottleneck analyzer.
  - `optimization_engine/`: Contains header files for the optimization engine.
  - `visualization/`: Contains header files for the visualization and reporting.
  - `utils/`: Contains header files for utility functions and helper classes.

- `tests/`: Contains the unit tests and integration tests for the project.
  - `unit_tests/`: Contains unit tests for individual components and functions.
  - `integration_tests/`: Contains integration tests for verifying the interaction between components.

- `third_party/`: Contains third-party libraries and dependencies used by the project.

## Development Workflow

To contribute to the NVMe-oF Benchmarking Suite, follow these steps:

1. Fork the project repository on GitHub.

2. Clone your forked repository to your local development environment:
   ```
   git clone https://github.com/muditbhargava66/nvmeof-benchmarking-suite.git
   ```

3. Create a new branch for your feature or bug fix:
   ```
   git checkout -b feature/your-feature-name
   ```

4. Make the necessary changes and additions to the codebase.

5. Write unit tests and integration tests to verify the correctness and functionality of your changes.

6. Ensure that all tests pass successfully:
   ```
   cd build
   make test
   ```

7. Commit your changes with descriptive commit messages:
   ```
   git commit -m "Add feature: your feature description"
   ```

8. Push your changes to your forked repository:
   ```
   git push origin feature/your-feature-name
   ```

9. Open a pull request on the main project repository, describing your changes and their purpose.

10. Engage in the code review process, addressing any feedback or comments from the project maintainers.

11. Once your pull request is approved, it will be merged into the main project repository.

## Coding Guidelines

To maintain a consistent and readable codebase, please adhere to the following coding guidelines:

- Follow the C++17 standard and use modern C++ features when applicable.
- Use meaningful and descriptive names for variables, functions, and classes.
- Write clear and concise comments to explain complex or non-obvious code sections.
- Adhere to the existing code style and formatting conventions used in the project.
- Handle errors and exceptions gracefully, providing informative error messages.
- Ensure proper resource management and cleanup to avoid memory leaks or resource exhaustion.
- Write unit tests for individual components and functions to verify their correctness.
- Write integration tests to validate the interaction between different components.
- Keep the codebase modular and maintainable, promoting code reuse and separation of concerns.

## Documentation

When contributing to the project, it's important to maintain and update the documentation accordingly. The project documentation includes:

- **User Guide**: Provides instructions for installing, configuring, and using the benchmarking suite.
- **Developer Guide**: Offers guidance for developers contributing to the project, including the architecture, code structure, and development workflow.
- **API Reference**: Documents the public APIs and interfaces exposed by the project.

When making changes to the codebase, ensure that the corresponding documentation is updated to reflect the modifications. This includes updating function and class descriptions, adding examples or usage instructions, and providing any necessary information for users and developers.

## Continuous Integration and Deployment

The NVMe-oF Benchmarking Suite uses a continuous integration and deployment (CI/CD) pipeline to ensure code quality and facilitate automated testing and deployment. The CI/CD pipeline is triggered whenever changes are pushed to the main project repository.

The CI/CD pipeline performs the following tasks:

1. Builds the project using the specified build system (e.g., CMake).

2. Runs the unit tests and integration tests to verify the correctness of the codebase.

3. Generates code coverage reports to assess the test coverage of the project.

4. Performs static code analysis to identify potential issues or code smells.

5. Deploys the project artifacts (binaries, libraries, documentation) to the designated deployment environment.

As a developer, you can monitor the status of the CI/CD pipeline and view the results of the automated tests and code analysis. If any issues are detected, you will be notified, and you should address them before proceeding with the merge or deployment.

## Issue Tracking and Bug Reporting

If you encounter any issues, bugs, or have feature requests, please use the project's issue tracking system on GitHub. When opening an issue, provide detailed information about the problem, including steps to reproduce it, expected behavior, and actual behavior.

When reporting a bug, include the following information:

- Version of the NVMe-oF Benchmarking Suite you are using.
- Operating system and environment details.
- Steps to reproduce the bug.
- Error messages or stack traces, if applicable.
- Any additional relevant information that can help in identifying and fixing the issue.

The project maintainers will review and prioritize the issues based on their severity and impact. You can also contribute to the project by helping to resolve open issues or implementing requested features.

## Conclusion

Thank you for your interest in contributing to the NVMe-oF Benchmarking Suite! By following the guidelines outlined in this Developer Guide, you can effectively contribute to the project, improve its functionality, and help in advancing the state of NVMe-oF performance benchmarking.

If you have any questions or need further assistance, please don't hesitate to reach out to the project maintainers or the developer community.

Happy coding and happy benchmarking!

---