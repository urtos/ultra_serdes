cmake_minimum_required(VERSION 3.16)
project(ultra_serdesTool C)

option(TARGET_ARCH "目标架构 (x86/arm/riscv/aarch64)" "x86")

include(cmake/toolchain.cmake)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${TARGET_ARCH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${TARGET_ARCH})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Wall -Wextra -Werror)
else()
    add_compile_options(-O2 -Wall -Wextra)
endif()

add_library(ultra_serdes_core STATIC
    src/ultra_serdes.c
)
target_include_directories(ultra_serdes_core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(ultra_serdes
    main.c
)
target_link_libraries(ultra_serdes
    PRIVATE
    ultra_serdes_core
)

enable_testing()

file(GLOB TEST_SOURCES "tests/*.c")
add_executable(test_ultra_serdes
    ${TEST_SOURCES}
)
target_link_libraries(test_ultra_serdes
    PRIVATE
    ultra_serdes_core
)
target_include_directories(test_ultra_serdes
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(QEMU_RUNNER)
    add_test(NAME ultra_serdesTest-${TARGET_ARCH}
        COMMAND ${QEMU_RUNNER} -L ${SYSROOT_PATH} $<TARGET_FILE:test_ultra_serdes>
    )
else()
    add_test(NAME ultra_serdesTest-${TARGET_ARCH}
        COMMAND $<TARGET_FILE:test_ultra_serdes>
    )
endif()

set_tests_properties(ultra_serdesTest-${TARGET_ARCH}
    PROPERTIES
    TIMEOUT 10
    PASS_REGULAR_EXPRESSION "All tests passed"
)
