cmake_minimum_required(VERSION 3.0.0)
project(gqlmapi-server-proto VERSION 0.1.0)

include(CTest)
enable_testing()

add_subdirectory(gqlmapi)

add_executable(gqlmapi-server-proto main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
