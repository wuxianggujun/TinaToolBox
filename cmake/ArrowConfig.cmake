# 首先检查是否已经找到 Arrow
find_package(Arrow CONFIG QUIET)
find_package(Parquet CONFIG QUIET)

if (Arrow_FOUND AND Parquet_FOUND)
    message(STATUS "Found Arrow: ${Arrow_VERSION}")
    return()
endif ()

# Arrow configuration
set(ARROW_BUILD_SHARED True)
set(ARROW_DEPENDENCY_SOURCE "SYSTEM")
set(ARROW_SIMD_LEVEL NONE)

# Arrow features
set(ARROW_ACERO ON)
set(ARROW_PARQUET ON)
set(ARROW_IPC ON)
set(ARROW_DATASET ON)
set(ARROW_FILESYSTEM ON)
set(ARROW_COMPUTE ON)

# 避免重复构建
if (NOT TARGET arrow_shared)

    include(FetchContent)
    FetchContent_Declare(Arrow
            GIT_REPOSITORY https://github.com/apache/arrow.git
            GIT_TAG apache-arrow-17.0.0
            SOURCE_SUBDIR cpp
            OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(Arrow)

    # Fix config header location
    file(INSTALL "${arrow_BINARY_DIR}/src/arrow/util/config.h"
            DESTINATION "${arrow_SOURCE_DIR}/cpp/src/arrow/util")

    target_include_directories(arrow_shared
            INTERFACE "$<BUILD_INTERFACE:${arrow_SOURCE_DIR}/cpp/src>"
    )
endif ()