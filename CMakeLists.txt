cmake_minimum_required(VERSION 3.10)

# 项目名称和语言设置（C/C++混编项目）
project(rk3506_smart_panel LANGUAGES C CXX)

# 生成 compile_commands.json 供 clangd 使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C 语言标准设置：使用 C11 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# C++ 语言标准设置：使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 工具链配置 ---
# 设置交叉编译工具链目录（RK3506 平台）
set(TOOLCHAIN_DIR $ENV{RK3506_TOOLCHAIN_DIR})
if(NOT TOOLCHAIN_DIR)
    set(TOOLCHAIN_DIR /home/gm/Workspace/linux_sdk/luckfox_rk3506_sdk/buildroot/output/rockchip_rk3506_luckfox/host)
endif()

# 设置编译器路径
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-buildroot-linux-gnueabihf-g++)
set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-buildroot-linux-gnueabihf-gcc)

# 设置系统根目录
set(CMAKE_SYSROOT ${TOOLCHAIN_DIR}/arm-buildroot-linux-gnueabihf/sysroot)

# --- 头文件搜索路径 ---
include_directories(
    # 系统库头文件
    ${CMAKE_SYSROOT}/usr/include/lvgl
    ${CMAKE_SYSROOT}/usr/include/lvgl/lv_drivers
    ${CMAKE_SYSROOT}/usr/include/c++/12.4.0
    # 项目头文件
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/app
    ${PROJECT_SOURCE_DIR}/include/hal
    ${PROJECT_SOURCE_DIR}/include/sensor
    ${PROJECT_SOURCE_DIR}/include/service
    # 源码头文件（包含EEZ Studio生成的UI代码）
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/ui
    # 第三方库头文件
    ${PROJECT_SOURCE_DIR}/third_party/rk/lvgl8
    ${PROJECT_SOURCE_DIR}/third_party/rk/common
    ${PROJECT_SOURCE_DIR}/third_party/rk/sys
)

# --- 宏定义 ---
add_definitions(-DUSE_RK3506=1)
add_definitions(-DUSE_DRM=1 -DUSE_EVDEV=1 -DUSE_RGA=1)

# --- 编译器标志 ---
# 禁用 GCC 7.1 ABI 变更警告（psabi）
add_compile_options(-Wno-psabi)

# --- 收集源文件 ---
# 第三方库源文件（C语言）
aux_source_directory(${PROJECT_SOURCE_DIR}/third_party/rk/lvgl8 SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/third_party/rk/common SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/third_party/rk/sys SRCS)

# 项目源文件（C/C++混编）
aux_source_directory(${PROJECT_SOURCE_DIR}/src/app SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/hal SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/sensor SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/ui SRCS)

# 主程序入口
list(APPEND SRCS ${PROJECT_SOURCE_DIR}/src/main.cpp)

# --- 创建可执行文件 ---
add_executable(${PROJECT_NAME} ${SRCS})

# --- 链接外部库 ---
target_link_libraries(${PROJECT_NAME}
    # LVGL 图形库
    lvgl lv_drivers
    # 字体渲染
    freetype
    # 显示和输入
    drm evdev rga
    # 系统库
    pthread m
    # 音频
    asound
    # FFmpeg 音视频编解码
    avformat avcodec avutil swscale swresample
    # 网络库
    curl
    # MQTT 客户端
    paho-mqttpp3 paho-mqtt3c
)

# --- 自定义清理目标 ---
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} -E remove CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove Makefile
    COMMAND ${CMAKE_COMMAND} -E remove_directory CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove ${PROJECT_NAME}
    COMMENT "Clean all CMake generated files and ${PROJECT_NAME}"
)

# --- 编译选项 ---
# 设为 OFF 可在无开发板时编译（跳过自动部署）
option(ENABLE_AUTO_DEPLOY "Enable auto-deploy to target board" OFF)

# --- 安装配置 ---
set(OVERLAY_ROOT_DIR $ENV{RK3506_OVERLAY_DIR})
if(NOT OVERLAY_ROOT_DIR)
    set(OVERLAY_ROOT_DIR "/home/gm/Workspace/linux_sdk/luckfox_rk3506_sdk/overlay/root")
endif()

install(TARGETS ${PROJECT_NAME}
    DESTINATION ${OVERLAY_ROOT_DIR}
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
)

# --- 自动传输到开发板 ---
if(ENABLE_AUTO_DEPLOY)
    # 配置目标开发板 IP 和密码
    set(TARGET_IP "192.168.5.71")
    set(TARGET_USER "root")
    set(TARGET_PATH "/root")
    set(TARGET_PASS "luckfox")

    # 添加编译后自动传输命令
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND echo "[Deploy] Stopping remote process..."
        COMMAND sshpass -p ${TARGET_PASS} ssh -o StrictHostKeyChecking=no -o ConnectTimeout=5 ${TARGET_USER}@${TARGET_IP} "killall ${PROJECT_NAME} 2>/dev/null || true"
        COMMAND echo "[Deploy] Transferring ${PROJECT_NAME} to ${TARGET_IP}:${TARGET_PATH}..."
        COMMAND sshpass -p ${TARGET_PASS} scp -o StrictHostKeyChecking=no -o ConnectTimeout=10 ${PROJECT_NAME} ${TARGET_USER}@${TARGET_IP}:${TARGET_PATH}/
        COMMAND echo "[Deploy] Transfer complete!"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Auto-deploy to target board after build"
        VERBATIM
    )

    # --- 编译、传输并运行目标（可选） ---
    add_custom_target(run
        COMMAND echo "[Run] Starting ${PROJECT_NAME} on ${TARGET_IP}..."
        COMMAND sshpass -p ${TARGET_PASS} ssh -o StrictHostKeyChecking=no ${TARGET_USER}@${TARGET_IP} "cd ${TARGET_PATH} && ./${PROJECT_NAME} &"
        COMMENT "Run ${PROJECT_NAME} on target board"
        VERBATIM
    )
else()
    # 禁用部署时的提示信息
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND echo "[Build] ${PROJECT_NAME} built successfully (auto-deploy disabled)"
        COMMENT "Auto-deploy is disabled. Use 'cmake -DENABLE_AUTO_DEPLOY=ON ..' to enable."
        VERBATIM
    )
endif()
