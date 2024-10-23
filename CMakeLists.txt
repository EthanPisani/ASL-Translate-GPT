cmake_minimum_required(VERSION 3.0)
project(YourProjectName)

# Compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenCV REQUIRED) # Assuming you have installed OpenCV and it's findable by CMake

# Include FetchContent for CPR
include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 0817715923c9705e68994eb52ef9df3f6845beba)
FetchContent_MakeAvailable(cpr)

# Include directories
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ./src
    ./libs/onnxruntime-linux-x64-1.16.1/include
    ./libs/onnxruntime/include
    ${cpr_SOURCE_DIR}/include # Include directory for CPR
)

# Library directories
link_directories(
    ./libs/onnxruntime-linux-x64-1.16.1/lib
)

# Source files
file(GLOB SRC_FILES ./src/*.cpp)
# Create an executable
add_executable(main main.cpp ${SRC_FILES}) # Include all source files here

# Link libraries
target_link_libraries(main
    ${OpenCV_LIBS} # Link with OpenCV libraries
    onnxruntime
    cpr::cpr
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
)


# Set the output directory for executables
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
