# cmake版本
cmake_minimum_required(VERSION 3.9.4)

# 项目名
project(libcommon VERSION 1.0.0)

# Include files
FILE(GLOB all_headers "*.h*")
source_group("include" FILES ${all_headers})

# Source files
FILE(GLOB all_srcs "*.c*")
source_group("src" FILES ${all_srcs})

# 设置
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
set(CMAKE_SUPPRESS_REGENERATION true)

# 第三方类库
include_directories(3rd/libuv/include)

# 生成静态库
add_library(libcommon STATIC ${all_headers} ${all_srcs})

# 添加单元测试项目
add_subdirectory(test)