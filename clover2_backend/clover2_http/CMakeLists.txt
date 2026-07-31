cmake_minimum_required(VERSION 3.20)
project(clover2_http)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CLOVER2_HTTP_BUILD_TESTS "Build unit tests" ON)
option(CLOVER2_HTTP_BUILD_EXAMPLES "Build examples" ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)

file(GLOB_RECURSE SRC_FILES "./src/*.cpp")
add_library(clover2_http_core STATIC ${SRC_FILES})

target_include_directories(clover2_http_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(clover2_http_core
    PUBLIC
        Boost::boost
        Boost::system
        nlohmann_json::nlohmann_json
)

if(CLOVER2_HTTP_BUILD_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()

    add_executable(test_router
        test/test_router.cpp
    )

    target_link_libraries(test_router
        PRIVATE
            clover2_http_core
            GTest::gtest
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(test_router
        PROPERTIES
            LABELS "unit"
            DISCOVERY_TIMEOUT 30
    )
endif()

if(CLOVER2_HTTP_BUILD_EXAMPLES)
    add_executable(example
        examples/example.cpp
    )

    target_link_libraries(example
        PRIVATE
            clover2_http_core
    )
endif()
