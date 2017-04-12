# 打印当前目录
message("测试工程当前目录: "  ${CMAKE_CURRENT_SOURCE_DIR})

# 需包含的头文件
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/gtest/include/gtest
    $ENV{KDS_MOBILE_STOCK_HOME}/3rd-soft/include/googletest1.8.0/include
    /usr/local/include
    /usr/include
)

# 需包含的库文件
link_directories(
    ${PROJECT_BINARY_DIR}/lib
    $ENV{KDS_MOBILE_STOCK_HOME}/3rd-soft/lib
    /usr/local/lib
    /usr/lib
)

find_package(Threads)

AUX_SOURCE_DIRECTORY(. DIR_SRC_TESTS)
add_executable(test_sult ${DIR_SRC_TESTS})
TARGET_LINK_LIBRARIES(test_sult uuid gtest gtest_main libsrc.a)

TARGET_LINK_LIBRARIES(test_sult ${CMAKE_THREAD_LIBS_INIT})
