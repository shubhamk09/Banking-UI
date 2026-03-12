cmake_minimum_required(VERSION 3.16)

# Create library for Communications module
add_library(communications_lib
    inc/BankingSocket.hpp
    inc/CommunicationInitializer.hpp
    BankingSocket.cpp
    CommunicationInitializer.cpp
)

# Find required Qt packages
find_package(Qt6 REQUIRED COMPONENTS Network)

# Set include directories
target_include_directories(communications_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# Link Qt libraries
target_link_libraries(communications_lib
    PRIVATE
        Qt6::Network
)

# Set C++ standard
target_compile_features(communications_lib PRIVATE cxx_std_17)