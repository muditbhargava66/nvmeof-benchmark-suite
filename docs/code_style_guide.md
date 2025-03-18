# NVMe-oF Benchmarking Suite Code Style Guide

This document outlines the coding standards and style guidelines for the NVMe-oF Benchmarking Suite project. Following these guidelines ensures consistency, readability, and maintainability of the codebase.

## 1. General Guidelines

### 1.1. Consistency

- Be consistent with the existing codebase.
- If you're modifying an existing file, follow the style of that file.
- If you're creating a new file, follow the guidelines in this document.

### 1.2. Formatting

- Use 4 spaces for indentation (no tabs).
- Line length should not exceed 100 characters.
- Use Unix-style line endings (LF).
- End each file with a newline.
- Remove trailing whitespace from all lines.
- Use whitespace to improve readability.

### 1.3. Source File Structure

Each source file should follow this structure:

1. License or copyright notice (if applicable)
2. Include guards for header files or module includes
3. Standard library includes
4. Third-party library includes
5. Project includes
6. Namespace declarations
7. Type declarations
8. Function implementations
9. Namespace closing

## 2. Naming Conventions

### 2.1. Namespaces

- Use lowercase for namespace names.
- Use descriptive names that reflect the functionality or module.

```cpp
namespace nvmeof {
namespace benchmarking {
// ...
}  // namespace benchmarking
}  // namespace nvmeof
```

### 2.2. Classes and Structs

- Use PascalCase for class and struct names.
- Use nouns for class and struct names.

```cpp
class WorkloadGenerator {
    // ...
};

struct WorkloadProfile {
    // ...
};
```

### 2.3. Functions and Methods

- Use PascalCase for public methods and functions.
- Use clear, descriptive verbs or verb phrases.

```cpp
void GenerateWorkload();
bool IsValid() const;
double GetProgress() const;
```

### 2.4. Variables

- Use camelCase for local variables and function parameters.
- Use snake_case with trailing underscore for member variables.

```cpp
void ProcessData(const std::string& inputData) {
    int itemCount = 0;
    // ...
}

class Example {
private:
    int count_;
    std::string name_;
};
```

### 2.5. Constants and Enumerations

- Use kPascalCase for constants and static const members.
- Use PascalCase for enumeration types and values.

```cpp
const int kMaxRetries = 3;

enum class BottleneckType {
    None,
    Cpu,
    Memory,
    Network
};
```

### 2.6. File Names

- Use lowercase with underscores for file names.
- Use .h for header files and .cpp for implementation files.

```
workload_generator.h
workload_generator.cpp
```

## 3. Comments and Documentation

### 3.1. File Headers

Each file should begin with a brief description:

```cpp
/**
 * @file workload_generator.h
 * @brief Defines the WorkloadGenerator class for generating NVMe-oF workloads.
 */
```

### 3.2. Class and Function Documentation

- Document all public classes and functions using Doxygen-style comments.
- Include a brief description, parameter descriptions, return value, and exceptions.

```cpp
/**
 * @brief Generates workloads according to the specified profile.
 * 
 * @param profile The workload profile to use
 * @return true if the workload was generated successfully, false otherwise
 * @throws std::invalid_argument if the profile is invalid
 */
bool GenerateWorkload(const WorkloadProfile& profile);
```

### 3.3. Code Comments

- Use comments to explain complex algorithms, non-obvious behavior, and design decisions.
- Avoid obvious comments that just repeat the code.
- Keep comments up-to-date with code changes.

```cpp
// Use binary search to find the optimal queue depth
// This is more efficient than a linear search for large ranges
int low = 1;
int high = 256;
while (low <= high) {
    // ...
}
```

## 4. C++ Specific Guidelines

### 4.1. Include Guards

Use `#pragma once` for include guards:

```cpp
#pragma once

// Header content here
```

### 4.2. Namespaces

- Use namespaces to organize code and prevent naming conflicts.
- Avoid using `using namespace` directives in header files.
- Always close namespaces with a comment:

```cpp
namespace nvmeof {
namespace benchmarking {

// Code here

}  // namespace benchmarking
}  // namespace nvmeof
```

### 4.3. Memory Management

- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) instead of raw pointers when ownership is involved.
- Use `const` references for function parameters when the function doesn't need to modify the parameter.
- Use move semantics when appropriate to avoid unnecessary copying.

### 4.4. Error Handling

- Use exceptions for exceptional situations, not for normal control flow.
- Document all exceptions that can be thrown by public methods.
- Use assertions (`assert`) to check for programming errors that should never happen.
- Use error codes or return values for expected failure conditions.

### 4.5. Class Design

- Follow the Single Responsibility Principle: each class should have a single responsibility.
- Use private member variables and provide accessors when needed.
- Initialize member variables in the constructor's initializer list.
- Define a virtual destructor for base classes.
- Mark methods as `const` when they don't modify the object's state.
- Use `override` and `final` keywords where appropriate.

## 5. Build and Testing Guidelines

### 5.1. CMake

- Use CMake for building the project.
- Organize CMake files hierarchically, with a top-level `CMakeLists.txt` and subdirectory files.
- Define project properties, dependencies, and targets clearly.

### 5.2. Testing

- Write unit tests for all non-trivial functions and classes.
- Use Google Test for unit testing.
- Aim for high test coverage, especially for critical components.
- Include both positive and negative test cases.
- Keep tests independent and repeatable.

## 6. Performance Considerations

- Use profiling tools to identify performance bottlenecks.
- Prefer stack allocation over heap allocation for small objects.
- Consider cache locality and memory access patterns.
- Avoid premature optimization; focus on correctness first, then optimize if needed.
- Document performance requirements and constraints.

## 7. Code Review Checklist

Before submitting code for review, check the following:

- Code compiles without warnings.
- All tests pass.
- Code follows the style guidelines.
- Documentation is complete and up-to-date.
- No unnecessary dependencies or unused code.
- No security vulnerabilities or performance issues.
- Error cases are handled appropriately.
- Code is as simple as possible, but no simpler.

## Conclusion

Following these guidelines will help maintain a consistent, readable, and maintainable codebase. Remember that these guidelines are meant to help, not hinder, development. Use your judgment and common sense when applying them.