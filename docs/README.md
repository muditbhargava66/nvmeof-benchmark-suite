# NVMe-oF Benchmarking Suite Documentation

Welcome to the documentation for the NVMe-oF Benchmarking Suite. This directory contains comprehensive documentation to help you install, use, and contribute to the project.

## Documentation Contents

### User Documentation

- [User Guide](user_guide.md) - Detailed instructions for end users
- [Installation Guide](INSTALL.md) - Step-by-step installation instructions
- [FAQ](FAQ.md) - Frequently asked questions
- [Troubleshooting](TROUBLESHOOTING.md) - Solutions for common issues

### Developer Documentation

- [Developer Guide](developer_guide.md) - Overall guide for developers
- [API Reference](api_reference.md) - Detailed API documentation
- [Contributing Guide](CONTRIBUTING.md) - How to contribute to the project
- [Code Style Guide](code_style_guide.md) - Coding standards and conventions
- [macOS Development Guide](macos_development.md) - Special instructions for macOS development

### Project Documentation

- [Changelog](../CHANGELOG.md) - History of changes and releases
- [Code of Conduct](CODE_OF_CONDUCT.md) - Community guidelines

## Documentation Format

All documentation is written in Markdown format for easy reading on GitHub and other Markdown viewers. The documentation aims to be:

- **Comprehensive**: Covering all aspects of the project
- **Clear**: Written in plain language
- **Current**: Regularly updated to reflect the latest changes
- **Concise**: Focused on providing valuable information

## Contributing to Documentation

We welcome contributions to improve the documentation! If you find any issues, have suggestions for improvements, or want to add new content, please follow these steps:

1. Fork the repository
2. Create a new branch for your changes
3. Make your edits to the documentation
4. Submit a pull request

Please follow these guidelines when contributing to documentation:

- Use clear, concise language
- Follow the existing style and format
- Include examples where appropriate
- Check for spelling and grammar errors
- Ensure links are working correctly

## Building Documentation

The documentation can be built using Doxygen for a more interactive experience:

```bash
cd build
cmake .. -DBUILD_DOCS=ON
make docs
```

The generated documentation will be available in the `build/docs/html` directory.

## Contact

If you have any questions about the documentation or need further assistance, please open an issue on the GitHub repository.