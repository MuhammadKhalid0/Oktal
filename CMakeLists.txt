cmake_minimum_required( VERSION 3.31 )
project( oktal LANGUAGES CXX )

set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_STANDARD_REQUIRED )
set( CMAKE_COMPILE_WARNING_AS_ERROR ON )
add_compile_options(-Wall -Wextra -Wconversion)

#   Create libraries

add_library( oktal )

target_include_directories( oktal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )

add_subdirectory( src )

#   Pull Helper Library

message( STATUS "Downloading AdvPT helper libraries..." )
include( FetchContent )
FetchContent_Declare(
    advpt-helpers
    URL https://teaching.pages.i10git.cs.fau.de/advpt-exercise-ws2526/assets/advpt-helpers.tar.gz
)
FetchContent_MakeAvailable( advpt-helpers )

#   Add test suite

option( OKTAL_BUILD_TESTSUITE "Build the Oktal test suite" ON )

if( OKTAL_BUILD_TESTSUITE )
    include( CTest )
    add_subdirectory( tests )
endif()

