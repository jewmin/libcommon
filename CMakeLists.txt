# cmake版本
cmake_minimum_required(VERSION 3.9.4)

# 项目名
project(libcommon)

# 源文件
aux_source_directory(. SRCS)

# 设置
set(CMAKE_BUILD_TYPE Debug)

# 第三方类库
include_directories(3rd/libuv/include)
link_directories(3rd/libuv/lib)
if (WIN32)
    link_libraries(libuv.lib)
endif()

# 生成方案
add_library(libcommon STATIC ${SRCS})

# 链接第三方类库
if (WIN32)
    target_link_libraries(libcommon libuv.lib)
endif()