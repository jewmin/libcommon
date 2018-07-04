cmake_minimum_required(VERSION 3.9.4)
project(libcommon)
aux_source_directory(. SRCS)
add_library(libcommon STATIC ${SRCS})