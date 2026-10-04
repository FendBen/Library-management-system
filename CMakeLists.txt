cmake_minimum_required(VERSION 3.16)
project(LibraryManager CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra)

add_library(lms_core
  src/util/Json.cpp
  src/util/Date.cpp
  src/util/Text.cpp
  src/storage/JsonStorage.cpp
  src/service/LibraryService.cpp
)
target_include_directories(lms_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(library_manager src/main.cpp src/cli/CliApp.cpp)
target_link_libraries(library_manager PRIVATE lms_core)

enable_testing()
add_executable(lms_tests tests/test_main.cpp)
target_link_libraries(lms_tests PRIVATE lms_core)
add_test(NAME lms_tests COMMAND lms_tests)
