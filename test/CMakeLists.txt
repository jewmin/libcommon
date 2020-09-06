# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# 项目名
PROJECT(unittest VERSION 1.0)

# 添加源文件
SET(test_srcs
	${PROJECT_SOURCE_DIR}/Main.cc
	${PROJECT_SOURCE_DIR}/AllocatorTest.cc
	${PROJECT_SOURCE_DIR}/CExpectionTest.cc
	${PROJECT_SOURCE_DIR}/CObjectTest.cc
	${PROJECT_SOURCE_DIR}/LocalTimeTest.cc
	${PROJECT_SOURCE_DIR}/MutexTest.cc
	${PROJECT_SOURCE_DIR}/RefCountedObjectTest.cc
	${PROJECT_SOURCE_DIR}/StringTest.cc
	${PROJECT_SOURCE_DIR}/ThreadTest.cc
	${PROJECT_SOURCE_DIR}/TimeStampTest.cc
)

# 生成可执行文件
ADD_EXECUTABLE(unittest-static ${test_srcs})
ADD_EXECUTABLE(unittest ${test_srcs})
TARGET_LINK_LIBRARIES(unittest-static common-static ${GTEST_LIBRARIES})
TARGET_LINK_LIBRARIES(unittest common ${GTEST_LIBRARIES})
IF(WINDOWS)
	TARGET_LINK_LIBRARIES(unittest-static ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
	TARGET_LINK_LIBRARIES(unittest ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
	TARGET_COMPILE_DEFINITIONS(unittest PRIVATE USING_COMMON_SHARED)
ENDIF()