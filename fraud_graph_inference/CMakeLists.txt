cmake_minimum_required(VERSION 3.24)
project(FraudGraphInference VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization for extreme low-latency AI inference
if(MSVC)
    add_compile_options(/O2 /arch:AVX2 /favor:INTEL64)
else()
    add_compile_options(-O3 -march=native -ffast-math)
endif()

# Source definitions
set(FRAUD_SOURCES
    src/LargeTabularModel.cpp
    src/GraphNeuralNetwork.cpp
    src/TensorRT_Optimizer.cpp
    src/AsyncRiskScorer.cpp
)

add_library(fraud_inference_lib STATIC ${FRAUD_SOURCES})

# In a real environment, you would link ONNX Runtime and TensorRT here:
# find_package(ONNXRuntime REQUIRED)
# find_package(TensorRT REQUIRED)
# target_link_libraries(fraud_inference_lib PRIVATE ONNXRuntime::onnxruntime TensorRT::nvinfer)

target_include_directories(fraud_inference_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(chronorisk_scorer src/AsyncRiskScorer.cpp)
target_link_libraries(chronorisk_scorer PRIVATE fraud_inference_lib Threads::Threads)
