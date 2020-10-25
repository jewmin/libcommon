# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

# 项目名
PROJECT(common_test VERSION 1.0)

# 添加源文件
SET(srcs
	${PROJECT_SOURCE_DIR}/Main.cc
	${PROJECT_SOURCE_DIR}/AllocatorTest.cc
	${PROJECT_SOURCE_DIR}/CExpectionTest.cc
	${PROJECT_SOURCE_DIR}/CListTest.cc
	${PROJECT_SOURCE_DIR}/CObjectTest.cc
	${PROJECT_SOURCE_DIR}/LocalTimeTest.cc
	${PROJECT_SOURCE_DIR}/MutexTest.cc
	${PROJECT_SOURCE_DIR}/RefCountedObjectTest.cc
	${PROJECT_SOURCE_DIR}/StringTest.cc
	${PROJECT_SOURCE_DIR}/ThreadTest.cc
	${PROJECT_SOURCE_DIR}/TimeStampTest.cc
	${PROJECT_SOURCE_DIR}/CBufferTest.cc
)

# 单元测试
ADD_EXECUTABLE(unittest-common-static ${srcs})
ADD_EXECUTABLE(unittest-common ${srcs})
TARGET_LINK_LIBRARIES(unittest-common-static common-static ${GTEST_LIBRARIES})
TARGET_LINK_LIBRARIES(unittest-common common ${GTEST_LIBRARIES})
IF(WINDOWS)
	TARGET_LINK_LIBRARIES(unittest-common-static ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
	TARGET_LINK_LIBRARIES(unittest-common ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
	TARGET_COMPILE_DEFINITIONS(unittest-common PRIVATE USING_COMMON_SHARED)
ENDIF()