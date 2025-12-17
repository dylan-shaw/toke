cmake_minimum_required(VERSION 3.20)

project(toke)

find_package(OpenMP REQUIRED COMPONENTS CXX)

option(TOKE_TESTS "Whether to enable building the unit tests."     OFF)
option(TOKE_TOOL  "Whether to build the tool for creating vocabs." OFF)

#==============#
# Main library #
#==============#

add_library(toke
  include/toke/decoder.h
  include/toke/encoder.h
  include/toke/error.h
  include/toke/filter.h
  src/decoder.c
  src/encoder.c
  src/error.c
  src/filter.c
)

target_include_directories(toke
  PUBLIC
    include
)

add_library(toke::toke ALIAS toke)

#=============#
# C++ library #
#=============#

add_library(toke_cxx
  include/toke/cxx_api.hpp
  src/cxx_api.cpp
)

target_include_directories(toke_cxx
  PUBLIC
    include
)

target_link_libraries(toke_cxx PUBLIC toke::toke)

add_library(toke::toke_cxx ALIAS toke_cxx)

#=========================#
# Tool for building vocab #
#=========================#

if(TOKE_TOOL)

  add_executable(toker
    tool/main.cpp
    tool/options.h
    tool/options.cpp
    tool/file_set.h
    tool/file_set.cpp
    tool/file_visitor.h
    tool/initializer.h
    tool/initializer.cpp
    tool/pair_counter.h
    tool/pair_counter.cpp
    tool/vocab.h
    tool/vocab.cpp
    tool/utf8.h
  )

  target_link_libraries(toker
        PRIVATE
        toke::toke_cxx
        OpenMP::OpenMP_CXX
  )

  target_compile_features(toker PRIVATE cxx_std_20)

endif()

#==============#
# Unit testing #
#==============#

if (TOKE_TESTS)

    find_package(GTest CONFIG REQUIRED)

    add_executable(toke_tests
      testing/encoder.cpp
      testing/decoder.cpp
      testing/filter.cpp
    )

    target_link_libraries(toke_tests
            PRIVATE
            toke::toke_cxx
            GTest::gtest
            GTest::gtest_main
    )

    set_target_properties(toke_tests
      PROPERTIES
        OUTPUT_NAME run_tests
    )

    add_test(NAME TokeTests COMMAND $<TARGET_FILE:toke_tests>)

    enable_testing()

endif ()
