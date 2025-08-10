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

    # QEMU 用户模式模拟器
    set(QEMU_RUNNER "qemu-arm")

    # 可扩展其他架构（如riscv）
elseif(TARGET_ARCH STREQUAL "riscv")
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR riscv64)
    set(CMAKE_C_COMPILER riscv64-linux-gnu-gcc)
    set(QEMU_RUNNER "qemu-riscv64")

else()
    message(FATAL_ERROR "不支持的架构: ${TARGET_ARCH}，可选值：x86/arm/riscv")
endif()

# 交叉编译时的查找路径配置（优先目标架构的库和头文件）
if(NOT TARGET_ARCH STREQUAL "x86")
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)   # 不搜索目标系统的程序
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)    # 只搜索目标系统的库
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)    # 只搜索目标系统的头文件
endif()
