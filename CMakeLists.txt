# cmake版本
cmake_minimum_required(VERSION 3.7.2)

# 项目名
project(libcommon VERSION 1.0.1)

# Include files
FILE(GLOB all_headers "*.h*")
source_group("include" FILES ${all_headers})

# Source files
FILE(GLOB all_srcs "*.c*")
source_group("src" FILES ${all_srcs})

# 设置
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_SUPPRESS_REGENERATION true)
add_definitions(-DUSE_JEMALLOC)

if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DJEMALLOC_EXPORT=)
    add_definitions(-DJEMALLOC_DEBUG)
    add_definitions(-DUSE_VLD)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=c++11)
endif()

# 第三方类库
include_directories(3rd/libuv/include)
if (MSVC)
    include_directories(3rd/jemalloc/include)
    include_directories(3rd/jemalloc/include/msvc_compat)
    include_directories(3rd/vld/include)
endif()

# 生成静态库
if (MSVC)
    add_library(libcommon STATIC ${all_headers} ${all_srcs})
else()
    add_library(common STATIC ${all_headers} ${all_srcs})
endif()

# 添加单元测试项目
add_subdirectory(test)
add_subdirectory(echo_server)
add_subdirectory(echo_client)