cmake_minimum_required(VERSION 3.20)
project(CinemaOOP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_COVERAGE "Enable coverage flags" OFF)

add_library(cinema_core
    src/application/cinema_service.cpp
    src/application/query_service.cpp
    src/infrastructure/file_repositories.cpp
)

target_include_directories(cinema_core PUBLIC src)

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cinema_core PUBLIC -O0 -g --coverage)
    target_link_options(cinema_core PUBLIC --coverage)
endif()

add_executable(cinema_app
    src/main.cpp
    src/presentation/cli.cpp
)

target_link_libraries(cinema_app PRIVATE cinema_core)

enable_testing()

add_executable(unit_tests tests/unit_tests.cpp)
target_link_libraries(unit_tests PRIVATE cinema_core)
add_test(NAME unit_tests COMMAND unit_tests)

add_executable(integration_tests tests/integration_tests.cpp)
target_link_libraries(integration_tests PRIVATE cinema_core)
add_test(NAME integration_tests COMMAND integration_tests)

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(unit_tests PUBLIC -O0 -g --coverage)
    target_link_options(unit_tests PUBLIC --coverage)
    target_compile_options(integration_tests PUBLIC -O0 -g --coverage)
    target_link_options(integration_tests PUBLIC --coverage)
endif()