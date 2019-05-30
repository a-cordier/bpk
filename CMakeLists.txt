cmake_minimum_required(VERSION 3.10.2)

project(bpk)

get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if (GENERATOR_IS_MULTI_CONFIG)
  set(CMAKE_BUILD_TYPE)
elseif (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-conan")

include(conan)

conan_cmake_run(
        REQUIRES
        boost_program_options/1.69.0@bincrafters/stable
        boost_filesystem/1.69.0@bincrafters/stable
        boost_regex/1.69.0@bincrafters/stable
        CMAKE_TARGETS
        BASIC_SETUP
        KEEP_RPATHS
        BUILD missing
)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        CONAN_PKG::boost_program_options
        CONAN_PKG::boost_filesystem
        CONAN_PKG::boost_regex
)


