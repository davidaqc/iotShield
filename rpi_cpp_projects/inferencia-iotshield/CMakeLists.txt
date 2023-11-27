cmake_minimum_required(VERSION 3.13.1)

set(PICO_BOARD pico_w)

set(MODEL_FOLDER .)
set(EI_SDK_FOLDER edge-impulse-sdk)

include(pico_sdk_import.cmake)

project(pico_anomaly_detection_project C CXX ASM)
set(CMAKE_C_STANDARD 11) 
set(CMAKE_CXX_STANDARD 17) 
if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_executable(anomaly_detection
    source/src/main.cpp
    source/src/mqtt_module.cpp
)

include(${MODEL_FOLDER}/edge-impulse-sdk/cmake/utils.cmake)

target_link_libraries(anomaly_detection
		pico_stdlib
                hardware_adc
                pico_cyw43_arch_lwip_threadsafe_background
                pico_stdlib
                pico_lwip_mbedtls
                pico_mbedtls
                pico_lwip_mqtt)

# enable usb output, disable uart output
pico_enable_stdio_usb(anomaly_detection 1)
pico_enable_stdio_uart(anomaly_detection 0)

target_include_directories(anomaly_detection PRIVATE
    ${MODEL_FOLDER}
    ${MODEL_FOLDER}/classifer
    ${MODEL_FOLDER}/tflite-model
    ${MODEL_FOLDER}/model-parameters
    ${MODEL_FOLDER}/source/include
)

target_include_directories(anomaly_detection PRIVATE
    ${EI_SDK_FOLDER}
    ${EI_SDK_FOLDER}/third_party/ruy
    ${EI_SDK_FOLDER}/third_party/gemmlowp
    ${EI_SDK_FOLDER}/third_party/flatbuffers/include
    ${EI_SDK_FOLDER}/third_party
    ${EI_SDK_FOLDER}/tensorflow
    ${EI_SDK_FOLDER}/dsp
    ${EI_SDK_FOLDER}/classifier
    ${EI_SDK_FOLDER}/CMSIS/NN/Include
    ${EI_SDK_FOLDER}/CMSIS/DSP/PrivateInclude
    ${EI_SDK_FOLDER}/CMSIS/DSP/Include
    ${EI_SDK_FOLDER}/CMSIS/Core/Include
)

include_directories(${INCLUDES})

# find model source files
RECURSIVE_FIND_FILE(MODEL_FILES "${MODEL_FOLDER}/tflite-model" "*.cpp")
RECURSIVE_FIND_FILE(SOURCE_FILES "${EI_SDK_FOLDER}" "*.cpp")
RECURSIVE_FIND_FILE(CC_FILES "${EI_SDK_FOLDER}" "*.cc")
RECURSIVE_FIND_FILE(S_FILES "${EI_SDK_FOLDER}" "*.s")
RECURSIVE_FIND_FILE(C_FILES "${EI_SDK_FOLDER}" "*.c")
list(APPEND SOURCE_FILES ${S_FILES})
list(APPEND SOURCE_FILES ${C_FILES})
list(APPEND SOURCE_FILES ${CC_FILES})
list(APPEND SOURCE_FILES ${MODEL_FILES})

# add all sources to the project
target_sources(anomaly_detection PRIVATE ${SOURCE_FILES})

pico_add_extra_outputs(anomaly_detection)
