# cmake版本
cmake_minimum_required(VERSION 3.9.4)

# 项目名
project(echo_server VERSION 1.0.0)

# Include files
FILE(GLOB all_headers "*.h*")
source_group("include" FILES ${all_headers})

# Source files
FILE(GLOB all_srcs "*.c*")
source_group("src" FILES ${all_srcs})

# 设置
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_SUPPRESS_REGENERATION true)

if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=c++11)
endif()

# 第三方类库
include_directories(${libcommon_SOURCE_DIR} ${libcommon_SOURCE_DIR}/3rd/libuv/include)
link_directories(${libcommon_SOURCE_DIR}/3rd/libuv/lib)

# 生成可执行文件
add_executable(${PROJECT_NAME} ${all_headers} ${all_srcs})
if (WIN32)
    target_link_libraries(${PROJECT_NAME} libcommon libuv advapi32 iphlpapi psapi ws2_32 userenv)
else()
    target_link_libraries(${PROJECT_NAME} libcommon libuv)
endif()