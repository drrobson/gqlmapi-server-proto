cmake_minimum_required(VERSION 3.0.0)
project(gqlmapi-server-proto VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

include(CTest)
enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/app)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/app)

find_package(oatpp CONFIG REQUIRED)
include_directories(${OATPP_INCLUDE_DIRS} gqlmapi/src/include )

add_subdirectory(gqlmapi)

add_executable(${PROJECT_NAME}-exe
        src/App.cpp
        src/controller/GqlQueryController.cpp
        src/resolver/MapiResolver.cpp)

target_link_libraries(${PROJECT_NAME}-exe
        PRIVATE oatpp::oatpp
        PRIVATE gqlmapi
        )

add_dependencies(${PROJECT_NAME}-exe gqlmapiCommon gqlmapi mapi_schema mapistub)

add_custom_command(TARGET ${PROJECT_NAME}-exe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/src/asset/index.html ${CMAKE_BINARY_DIR}/app/index.html)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
