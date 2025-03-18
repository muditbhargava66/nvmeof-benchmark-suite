# Add the benchmarking library
add_library(benchmarking STATIC
    benchmarking/workload_generator.cpp
    benchmarking/data_collector.cpp
    benchmarking/result_visualizer.cpp
)
target_include_directories(benchmarking
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
target_link_libraries(benchmarking
    PUBLIC
        Threads::Threads
        ${SPDK_LIBRARIES}
)
set_target_properties(benchmarking PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Add the bottleneck_analysis library
add_library(bottleneck_analysis STATIC
    bottleneck_analysis/system_profiler.cpp
    bottleneck_analysis/resource_monitor.cpp
    bottleneck_analysis/bottleneck_detector.cpp
)
target_include_directories(bottleneck_analysis
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
target_link_libraries(bottleneck_analysis
    PUBLIC
        Threads::Threads
)
set_target_properties(bottleneck_analysis PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Add the optimization_engine library
add_library(optimization_engine STATIC
    optimization_engine/config_knowledge_base.cpp
    optimization_engine/optimizer.cpp
    optimization_engine/config_applicator.cpp
)
target_include_directories(optimization_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
target_link_libraries(optimization_engine
    PUBLIC
        bottleneck_analysis
)
set_target_properties(optimization_engine PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Add the utils library
add_library(utils STATIC
    utils/nvmeof_utils.cpp
    utils/hardware_detection.cpp
)
target_include_directories(utils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
set_target_properties(utils PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Add the nvmeof_lib library
add_library(nvmeof_lib STATIC
    main.cpp
)
target_link_libraries(nvmeof_lib
    PUBLIC
        benchmarking
        bottleneck_analysis
        optimization_engine
        utils
)
target_include_directories(nvmeof_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
set_target_properties(nvmeof_lib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Add the nvmeof_benchmarking executable
add_executable(nvmeof_benchmarking
    main.cpp
)
target_link_libraries(nvmeof_benchmarking
    PRIVATE
        nvmeof_lib
)

# Add additional executables as needed
add_executable(nvmeof_analysis
    analysis.cpp
)
target_link_libraries(nvmeof_analysis
    PRIVATE
        nvmeof_lib
)

add_executable(nvmeof_visualizer
    visualizer.cpp
)
target_link_libraries(nvmeof_visualizer
    PRIVATE
        nvmeof_lib
)

# Install targets
install(TARGETS
    benchmarking
    bottleneck_analysis
    optimization_engine
    utils
    nvmeof_lib
    nvmeof_benchmarking
    nvmeof_analysis
    nvmeof_visualizer
    EXPORT nvmeof_benchmarking_suite-targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)