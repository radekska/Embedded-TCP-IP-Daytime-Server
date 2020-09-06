cmake_minimum_required(VERSION 3.15)
project(datetime_client_2_0)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(python)

add_executable(datetime_client_2_0 datetime_client.c)
