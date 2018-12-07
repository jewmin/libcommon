# cmake版本
cmake_minimum_required(VERSION 3.7.2)

# 项目名
project(echo_client VERSION 1.0.0)

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
    add_definitions(-DJEMALLOC_STATIC)
    add_definitions(-DUSE_VLD)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=c++11)
endif()

# 第三方类库
include_directories(${libcommon_SOURCE_DIR})
include_directories(${libcommon_SOURCE_DIR}/3rd/libuv/include)
if (MSVC)
    include_directories(${libcommon_SOURCE_DIR}/3rd/jemalloc/include)
    include_directories(${libcommon_SOURCE_DIR}/3rd/jemalloc/include/msvc_compat)
    include_directories(${libcommon_SOURCE_DIR}/3rd/vld/include)
endif()
link_directories(${libcommon_SOURCE_DIR}/3rd/libuv/lib)
if (MSVC)
    link_directories(${libcommon_SOURCE_DIR}/3rd/jemalloc/lib)
    link_directories(${libcommon_SOURCE_DIR}/3rd/vld/lib)
endif()

# 生成可执行文件
add_executable(${PROJECT_NAME} ${all_headers} ${all_srcs})
if (MSVC)
    target_link_libraries(${PROJECT_NAME} libcommon libuv jemalloc advapi32 iphlpapi psapi ws2_32 userenv vld)
else()
    target_link_libraries(${PROJECT_NAME} common uv jemalloc)
endif()