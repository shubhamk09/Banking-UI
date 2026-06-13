# Unit tests for Common library

add_subdirectory(${CMAKE_SOURCE_DIR}/External/googletest googletest)

add_executable(test_common_messagequeue
    MessageQueueTest.cpp
)

target_include_directories(test_common_messagequeue PRIVATE
    ${CMAKE_SOURCE_DIR}/Src/Common/inc
    ${CMAKE_SOURCE_DIR}/Src/Common/Interfaces
)

target_link_libraries(test_common_messagequeue
    PRIVATE GTest::gtest_main messagequeue_lib
)

include(GoogleTest)
gtest_discover_tests(test_common_messagequeue)
