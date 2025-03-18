# Add unit tests and integration tests subdirectories
add_subdirectory(unit_tests)
add_subdirectory(integration_tests)

# Add a target to run all tests
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS unit_tests integration_tests
    COMMENT "Running all tests"
)

# Add test coverage reporting if GCC or Clang is used
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
    
    if(ENABLE_COVERAGE)
        # Add coverage flags to compiler options
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
        
        # Find required tools
        find_program(GCOV_PATH gcov)
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)
        
        if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
            # Add custom target for generating coverage report
            add_custom_target(coverage
                # Clean previous coverage data
                COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters
                
                # Run tests
                COMMAND ${CMAKE_CTEST_COMMAND} --verbose
                
                # Capture coverage data
                COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --capture --output-file ${CMAKE_BINARY_DIR}/coverage.info
                
                # Remove coverage data for system headers
                COMMAND ${LCOV_PATH} --remove ${CMAKE_BINARY_DIR}/coverage.info '/usr/*' --output-file ${CMAKE_BINARY_DIR}/coverage.info
                
                # Generate HTML report
                COMMAND ${GENHTML_PATH} ${CMAKE_BINARY_DIR}/coverage.info --output-directory ${CMAKE_BINARY_DIR}/coverage_report
                
                # Output report location
                COMMAND ${CMAKE_COMMAND} -E echo "Coverage report available at: ${CMAKE_BINARY_DIR}/coverage_report/index.html"
                
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating code coverage report"
            )
        else()
            message(WARNING "Code coverage tools not found. Coverage reporting will not be available.")
        endif()
    endif()
endif()