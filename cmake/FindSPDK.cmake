# FindSPDK.cmake - Find SPDK libraries
#
# This module defines:
#  SPDK_FOUND - True if SPDK is found
#  SPDK_INCLUDE_DIRS - The SPDK include directories
#  SPDK_LIBRARIES - The SPDK libraries
#  SPDK_VERSION - The SPDK version string

# Define search paths
set(_SPDK_SEARCH_PATHS
    ${SPDK_DIR}
    $ENV{SPDK_DIR}
    /usr/local/spdk
    /opt/spdk
)

# Find SPDK headers
find_path(SPDK_INCLUDE_DIR
    NAMES spdk/nvme.h
    PATHS ${_SPDK_SEARCH_PATHS}
    PATH_SUFFIXES include
)

# If we're on macOS and using mock, look in the third_party directory
if(APPLE AND USE_MOCK_SPDK)
    find_path(SPDK_INCLUDE_DIR
        NAMES spdk/nvme.h
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdk_mock
        PATH_SUFFIXES include
        NO_DEFAULT_PATH
    )
endif()

# Find SPDK libraries - adjust this list based on what your project needs
set(_SPDK_REQUIRED_LIBS nvme bdev rpc accel thread util)

# Initialize the library list
set(SPDK_LIBRARIES "")

foreach(lib ${_SPDK_REQUIRED_LIBS})
    find_library(SPDK_${lib}_LIBRARY
        NAMES spdk_${lib}
        PATHS ${_SPDK_SEARCH_PATHS}
        PATH_SUFFIXES lib build/lib
    )
    
    if(SPDK_${lib}_LIBRARY)
        list(APPEND SPDK_LIBRARIES ${SPDK_${lib}_LIBRARY})
    endif()
    
    # Clean up the individual library variables
    mark_as_advanced(SPDK_${lib}_LIBRARY)
endforeach()

# For macOS with mock implementation
if(APPLE AND USE_MOCK_SPDK)
    # If using the mock implementation, set a single mock library
    find_library(SPDK_MOCK_LIBRARY
        NAMES spdk_mock
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdk_mock
        PATH_SUFFIXES lib
        NO_DEFAULT_PATH
    )
    
    if(SPDK_MOCK_LIBRARY)
        set(SPDK_LIBRARIES ${SPDK_MOCK_LIBRARY})
    endif()
    
    mark_as_advanced(SPDK_MOCK_LIBRARY)
endif()

# Try to get version information
if(SPDK_INCLUDE_DIR AND EXISTS "${SPDK_INCLUDE_DIR}/spdk/version.h")
    file(STRINGS "${SPDK_INCLUDE_DIR}/spdk/version.h" SPDK_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SPDK_VERSION_MAJOR[ \t]+[0-9]+$")
    file(STRINGS "${SPDK_INCLUDE_DIR}/spdk/version.h" SPDK_VERSION_MINOR_LINE REGEX "^#define[ \t]+SPDK_VERSION_MINOR[ \t]+[0-9]+$")
    file(STRINGS "${SPDK_INCLUDE_DIR}/spdk/version.h" SPDK_VERSION_PATCH_LINE REGEX "^#define[ \t]+SPDK_VERSION_PATCH[ \t]+[0-9]+$")
    
    string(REGEX REPLACE "^#define[ \t]+SPDK_VERSION_MAJOR[ \t]+([0-9]+)$" "\\1" SPDK_VERSION_MAJOR "${SPDK_VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SPDK_VERSION_MINOR[ \t]+([0-9]+)$" "\\1" SPDK_VERSION_MINOR "${SPDK_VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SPDK_VERSION_PATCH[ \t]+([0-9]+)$" "\\1" SPDK_VERSION_PATCH "${SPDK_VERSION_PATCH_LINE}")
    
    set(SPDK_VERSION "${SPDK_VERSION_MAJOR}.${SPDK_VERSION_MINOR}.${SPDK_VERSION_PATCH}")
elseif(APPLE AND USE_MOCK_SPDK)
    # For mock implementation, set a default version
    set(SPDK_VERSION "mock-1.0.0")
else()
    set(SPDK_VERSION "unknown")
endif()

# Set include directories
if(SPDK_INCLUDE_DIR)
    set(SPDK_INCLUDE_DIRS ${SPDK_INCLUDE_DIR})
endif()

# Standard find package handling
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPDK
    REQUIRED_VARS SPDK_INCLUDE_DIRS SPDK_LIBRARIES
    VERSION_VAR SPDK_VERSION
)

# Mark variables as advanced
mark_as_advanced(
    SPDK_INCLUDE_DIR
    SPDK_LIBRARIES
)

# Set SPDK_FOUND to true if SPDK is found
if(SPDK_INCLUDE_DIRS AND SPDK_LIBRARIES)
    set(SPDK_FOUND TRUE)
endif()
