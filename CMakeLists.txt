cmake_minimum_required(VERSION 3.13)
# project info
project(compiler
        VERSION 2024
        DESCRIPTION "ZheJiang University 2023~2024 Spring-Summer Compiler Project"
        LANGUAGES C CXX)

# CTest
enable_testing()

# settings
# set to OFF to enable C mode
set(CPP_MODE ON)
if(CPP_MODE)
  set(FB_EXT ".cc")
else()
  set(FB_EXT ".c")
endif()
message(STATUS "Flex/Bison generated source file extension: ${FB_EXT}")

# enable all warnings
if(MSVC)
  add_compile_options(/W3)
else()
  # disable warnings caused by old version of Flex
  add_compile_options(-Wall -Wno-register)
endif()

# find Flex/Bison
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# generate lexer/parser
file(GLOB_RECURSE L_SOURCES "src/*.l")
file(GLOB_RECURSE Y_SOURCES "src/*.y")
if(NOT (L_SOURCES STREQUAL "" AND Y_SOURCES STREQUAL ""))
  string(REGEX REPLACE ".*/(.*)\\.l" "${CMAKE_CURRENT_BINARY_DIR}/\\1.lex${FB_EXT}" L_OUTPUTS "${L_SOURCES}")
  string(REGEX REPLACE ".*/(.*)\\.y" "${CMAKE_CURRENT_BINARY_DIR}/\\1.tab${FB_EXT}" Y_OUTPUTS "${Y_SOURCES}")
  flex_target(Lexer ${L_SOURCES} ${L_OUTPUTS})
  bison_target(Parser ${Y_SOURCES} ${Y_OUTPUTS})
  add_flex_bison_dependency(Lexer Parser)
endif()
# add fmt
add_subdirectory(third_party/fmt EXCLUDE_FROM_ALL)

# project include directories
include_directories(include)
include_directories(third_party/fmt/include)

# all of C/C++ source files
file(GLOB_RECURSE C_SOURCES "src/*.c")
file(GLOB_RECURSE CXX_SOURCES "src/*.cpp")
file(GLOB_RECURSE CC_SOURCES "src/*.cc")
set(SOURCES ${C_SOURCES} ${CXX_SOURCES} ${CC_SOURCES}
            ${FLEX_Lexer_OUTPUTS} ${BISON_Parser_OUTPUT_SOURCE})

# executable
add_executable(compiler ${SOURCES})
set_target_properties(compiler PROPERTIES C_STANDARD 11 CXX_STANDARD 17)
target_link_libraries(compiler PRIVATE fmt::fmt-header-only)

# some possible build options you may use.
# you can specify whatever you prefer.

# disable rtti and exception.
# target_compile_options(compiler PRIVATE -fno-rtti -fno-exceptions)

# enable address sanitizer if you encounter segmentation fault.
# target_compile_options(compiler PRIVATE -O0 -g -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls)

# all test C/C++ sources files
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")
foreach (test_source ${TEST_SOURCES})
    # Create test suit
    get_filename_component(test_filename ${test_source} NAME)
    string(REPLACE ".cpp" "" test_name ${test_filename})
    message(STATUS "Create test suit: ${test_name}")

    # Add the test target separately and as part of "make check-tests".
    add_executable(${test_name} EXCLUDE_FROM_ALL ${test_source})
    target_sources(${test_name} PUBLIC ${SOURCES})
    # Set test target properties and dependencies.
    set_target_properties(${test_name}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
            COMMAND ${test_name}
            )

    # Add the test under CTest.
    add_test(${test_name} ${CMAKE_BINARY_DIR}/test/${test_name}) 

endforeach(test_source ${TEST_SOURCES})