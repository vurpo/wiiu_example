cmake_minimum_required(VERSION 3.2)
project(vurpo_example CXX)

add_executable(vurpo_example
   main.cpp)

wut_create_rpx(vurpo_example)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/vurpo_example.rpx"
        DESTINATION "${CMAKE_INSTALL_PREFIX}")
