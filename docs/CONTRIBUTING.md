# Contributing to NVMe-oF Benchmarking Suite

Thank you for your interest in contributing to the NVMe-oF Benchmarking Suite! This document outlines the process for contributing to the project and provides guidelines to ensure a smooth collaboration.

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [Development Workflow](#development-workflow)
4. [Submitting Changes](#submitting-changes)
5. [Code Style](#code-style)
6. [Testing](#testing)
7. [Documentation](#documentation)
8. [Issue Reporting](#issue-reporting)
9. [Feature Requests](#feature-requests)
10. [Community](#community)

## Code of Conduct

Please read and follow our [Code of Conduct](CODE_OF_CONDUCT.md) to ensure a positive and inclusive community environment.

## Getting Started

### Prerequisites

Before you begin, ensure you have the following installed:
- Git
- CMake (version 3.14 or higher)
- GCC or Clang compiler with C++17 support
- SPDK development libraries

### Setting Up the Development Environment

1. Fork the repository on GitHub.

2. Clone your fork locally:
   ```
   git clone https://github.com/your-username/nvmeof-benchmark-suite.git
   cd nvmeof-benchmark-suite
   ```

3. Add the original repository as an upstream remote:
   ```
   git remote add upstream https://github.com/muditbhargava66/nvmeof-benchmark-suite.git
   ```

4. Create a build directory and configure the project:
   ```
   mkdir build && cd build
   cmake ..
   ```

5. Build the project:
   ```
   make
   ```

6. Run the tests to ensure everything is working:
   ```
   make test
   ```

## Development Workflow

1. Create a new branch for your feature or bug fix:
   ```
   git checkout -b feature/your-feature-name
   ```
   or
   ```
   git checkout -b fix/issue-description
   ```

2. Make your changes, following the code style guidelines.

3. Add and commit your changes with a descriptive commit message:
   ```
   git add .
   git commit -m "Add feature: description of your feature"
   ```

4. Keep your branch up-to-date with the upstream repository:
   ```
   git fetch upstream
   git rebase upstream/main
   ```

5. Push your branch to your fork:
   ```
   git push origin feature/your-feature-name
   ```

## Submitting Changes

1. Ensure all tests pass:
   ```
   make test
   ```

2. Update documentation if necessary.

3. Submit a pull request (PR) to the main repository's `main` branch.

4. In your PR description:
   - Describe the changes you've made
   - Reference any relevant issues
   - Mention any breaking changes
   - Include information on how to test your changes

5. Participate in the code review process by addressing feedback.

## Code Style

Please follow the [Code Style Guide](code_style_guide.md) for this project. Key points include:

- Use 4 spaces for indentation (no tabs)
- Follow naming conventions:
  - `PascalCase` for classes, structs, and public methods
  - `camelCase` for local variables and function parameters
  - `snake_case_` with trailing underscore for member variables
- Add meaningful comments and documentation
- Keep lines under 100 characters

## Testing

- Write unit tests for all new functionality
- Ensure all existing tests pass before submitting a PR
- Tests should be placed in the appropriate directory under `tests/`
- Use the Google Test framework for unit tests
- Include both positive and negative test cases

To run tests:
```
cd build
make test
```

For more detailed output:
```
ctest -V
```

## Documentation

- Update documentation for any changed functionality
- Document all public APIs using Doxygen-style comments
- Include example usage where appropriate
- Update README.md if needed

## Issue Reporting

If you find a bug or have a suggestion:

1. Check if the issue already exists in the [Issues](https://github.com/muditbhargava66/nvmeof-benchmark-suite/issues)

2. If not, create a new issue, providing:
   - A clear title and description
   - Steps to reproduce the issue
   - Expected and actual behavior
   - System information (OS, compiler version, etc.)
   - Any relevant logs or screenshots

## Feature Requests

Feature requests are welcome! When suggesting a feature:

1. Describe the feature and its benefits
2. Explain how it fits into the project's scope
3. Consider implementation details if possible
4. Indicate if you're willing to implement it yourself

## Community

Join our community discussions:

- GitHub Discussions


## License

By contributing to this project, you agree that your contributions will be licensed under the project's license.

Thank you for contributing to the NVMe-oF Benchmarking Suite!