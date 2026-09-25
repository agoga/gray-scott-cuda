cmake_minimum_required(VERSION 3.25)

project(gray_scott LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_ARCHITECTURES 86)

include(FetchContent)

FetchContent_Declare(
    gif_h
    GIT_REPOSITORY https://github.com/charlietangora/gif-h.git
    GIT_TAG 05dd65662f03f10880070f1a673f271a267b2fe8
)
FetchContent_MakeAvailable(gif_h)

find_package(CUDAToolkit REQUIRED)
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.13.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(pybind11)

add_library(gray_scott_core STATIC
    src/cpu_simulator.cpp
    src/cuda_simulator.cu
    src/gif_writer.cpp
    src/initial_conditions.cpp
    src/simulation_runner.cpp
)
target_include_directories(gray_scott_core PUBLIC include ${gif_h_SOURCE_DIR})
target_link_libraries(gray_scott_core PUBLIC CUDA::cudart)

add_executable(gray_scott_cli
    src/main.cpp
)
target_include_directories(gray_scott_cli PRIVATE include)
target_link_libraries(gray_scott_cli PRIVATE gray_scott_core)

pybind11_add_module(_gray_scott
    src/python_module.cpp
)
target_link_libraries(_gray_scott PRIVATE gray_scott_core)
target_include_directories(_gray_scott PRIVATE include)

# Keep generated Python files under the build tree. The wrapper is copied beside
# the extension so the build output is itself importable with PYTHONPATH.
set(PYTHON_PACKAGE_DIR "${CMAKE_BINARY_DIR}/python/gray_scott")
file(COPY "${CMAKE_SOURCE_DIR}/python/gray_scott/__init__.py"
    DESTINATION "${PYTHON_PACKAGE_DIR}")
set_target_properties(_gray_scott PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PYTHON_PACKAGE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${PYTHON_PACKAGE_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${PYTHON_PACKAGE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PYTHON_PACKAGE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PYTHON_PACKAGE_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PYTHON_PACKAGE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PYTHON_PACKAGE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PYTHON_PACKAGE_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PYTHON_PACKAGE_DIR}"
)
