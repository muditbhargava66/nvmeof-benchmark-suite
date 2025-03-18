# Add unit test sources
set(UNIT_TEST_SOURCES
    # Benchmarking tests
    benchmarking/workload_generator_test.cpp
    benchmarking/data_collector_test.cpp
    benchmarking/result_visualizer_test.cpp
    
    # Bottleneck analysis tests
    bottleneck_analysis/system_profiler_test.cpp
    bottleneck_analysis/resource_monitor_test.cpp
    bottleneck_analysis/bottleneck_detector_test.cpp
    
    # Optimization engine tests
    optimization_engine/config_knowledge_base_test.cpp
    optimization_engine/optimizer_test.cpp
    optimization_engine/config_applicator_test.cpp
    
    # Utils tests
    utils/nvmeof_utils_test.cpp
    utils/hardware_detection_test.cpp
)

# Add the unit test executable
add_executable(unit_tests
    ${UNIT_TEST_SOURCES}
    main_test.cpp  # Contains test main function
)

# Link with the required libraries
target_link_libraries(unit_tests
    PRIVATE
        benchmarking
        bottleneck_analysis
        optimization_engine
        utils
        gtest
        gtest_main
        gmock
)

# Add include directories
target_include_directories(unit_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${gtest_SOURCE_DIR}/include
        ${gmock_SOURCE_DIR}/include
)

# Enable testing with GoogleTest
gtest_discover_tests(unit_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PROPERTIES
        TIMEOUT 30  # Timeout for each test in seconds
)

# Create individual test targets for test categories
add_custom_target(run_benchmarking_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "benchmarking" --verbose
    DEPENDS unit_tests
    COMMENT "Running benchmarking unit tests"
)

add_custom_target(run_bottleneck_analysis_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "bottleneck_analysis" --verbose
    DEPENDS unit_tests
    COMMENT "Running bottleneck analysis unit tests"
)

add_custom_target(run_optimization_engine_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "optimization_engine" --verbose
    DEPENDS unit_tests
    COMMENT "Running optimization engine unit tests"
)

add_custom_target(run_utils_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "utils" --verbose
    DEPENDS unit_tests
    COMMENT "Running utils unit tests"
)