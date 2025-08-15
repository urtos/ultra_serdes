# 根据 TARGET_ARCH 决定目标架构（默认x86）
if(NOT DEFINED TARGET_ARCH OR TARGET_ARCH STREQUAL "" OR TARGET_ARCH STREQUAL "OFF")
    set(TARGET_ARCH "x86")
endif()

# x86 架构：使用本地编译器，无需QEMU
if(TARGET_ARCH STREQUAL "x86")
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
    set(QEMU_RUNNER "")  # 空字符串表示直接运行

    # ARM 架构：使用交叉编译器和 qemu-arm
elseif(TARGET_ARCH STREQUAL "arm")
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)

    # 交叉编译器（根据实际安装路径调整）
    set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
    set(SYSROOT_PATH "/usr/arm-linux-gnueabihf")
    set(QEMU_RUNNER "qemu-arm")

elseif(TARGET_ARCH STREQUAL "riscv")
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR riscv64)
    set(CMAKE_C_COMPILER riscv64-linux-gnu-gcc)
    set(SYSROOT_PATH "/usr/riscv64-linux-gnu")
    set(QEMU_RUNNER "qemu-riscv64")

elseif(TARGET_ARCH STREQUAL "aarch64")
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    set(SYSROOT_PATH "/usr/aarch64-linux-gnu")
    set(QEMU_RUNNER "qemu-aarch64")

else()
    message(FATAL_ERROR "不支持的架构: ${TARGET_ARCH}，可选值：x86/arm/riscv")
endif()

if(NOT TARGET_ARCH STREQUAL "x86")
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)   # 不搜索目标系统的程序
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)    # 只搜索目标系统的库
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)    # 只搜索目标系统的头文件
endif()
