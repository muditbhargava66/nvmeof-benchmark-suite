cmake_minimum_required(VERSION 3.14)

# Check if we're on macOS
if(APPLE)
    message(STATUS "Detected macOS: Using SPDK mock implementation")
    add_subdirectory(spdk_mock)
    
    # Set variables to make the mock library available to the main project
    set(SPDK_FOUND TRUE PARENT_SCOPE)
    set(SPDK_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/spdk_mock/include" PARENT_SCOPE)
    set(SPDK_LIBRARIES spdk_mock PARENT_SCOPE)
else()
    # On Linux, try to find the real SPDK
    message(STATUS "Attempting to find real SPDK installation")
    
    # Check if SPDK_DIR is provided
    if(DEFINED ENV{SPDK_DIR})
        set(SPDK_DIR $ENV{SPDK_DIR})
        message(STATUS "Using SPDK from environment variable: ${SPDK_DIR}")
    else()
        set(SPDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/spdk")
        message(STATUS "Using SPDK from third_party directory: ${SPDK_DIR}")
    endif()
    
    # Check if SPDK exists at the specified location
    if(EXISTS "${SPDK_DIR}/include/spdk/nvme.h")
        set(SPDK_FOUND TRUE PARENT_SCOPE)
        set(SPDK_INCLUDE_DIRS "${SPDK_DIR}/include" PARENT_SCOPE)
        
        # Find SPDK libraries
        find_library(SPDK_NVME_LIBRARY spdk_nvme 
            PATHS "${SPDK_DIR}/build/lib" 
            NO_DEFAULT_PATH
        )
        
        if(SPDK_NVME_LIBRARY)
            set(SPDK_LIBRARIES ${SPDK_NVME_LIBRARY} PARENT_SCOPE)
            message(STATUS "Found SPDK libraries: ${SPDK_NVME_LIBRARY}")
        else()
            message(WARNING "SPDK libraries not found. You may need to build SPDK first.")
            set(SPDK_FOUND FALSE PARENT_SCOPE)
        endif()
    else()
        # SPDK not found, will need to be built or installed
        message(WARNING "SPDK not found at ${SPDK_DIR}. See README for installation instructions.")
        set(SPDK_FOUND FALSE PARENT_SCOPE)
    endif()
endif()

# Add other third-party libraries if needed
# add_subdirectory(googletest)  # Example for Google Test