cmake_minimum_required(VERSION 3.20)

project(toke)

find_package(OpenMP REQUIRED COMPONENTS CXX)

option(TOKE_TESTS  "Whether to enable building the unit tests."     OFF)
option(TOKE_TOOL   "Whether to build the tool for creating vocabs." OFF)
option(TOKE_TRAIN  "Whether to build the training library."          ON)
option(TOKE_PYTHON "Whether to build the Python bindings."           ON)

#==============#
# Main library #
#==============#

add_library(toke_core
  include/toke/decoder.h
  include/toke/encoder.h
  include/toke/error.h
  include/toke/filter.h
  include/toke/model.h
  src/decoder.c
  src/encoder.c
  src/error.c
  src/filter.c
  src/model.c
  src/vocab.h
  src/vocab.c
)

target_include_directories(toke_core
  PUBLIC
    include
)

add_library(toke::core ALIAS toke_core)

#==================#
# Training Library #
#==================#

if(TOKE_TRAIN OR TOKE_PYTHON)

  set(sources
    include/toke/train/dataset.h
    src/train/dataset.c
    src/train/memmap.h
  )

  if(UNIX)
    list(APPEND sources src/train/memmap_unix.c)
  else()
    message(FATAL_ERROR "platform not supported")
  endif()

  add_library(toke_train ${sources})

  target_include_directories(toke_train
    PUBLIC
      include
  )

  target_link_libraries(toke_train
    PUBLIC
      toke::core
  )

  add_library(toke::train ALIAS toke_train)

endif()

#================#
# Python Library #
#================#

if(TOKE_PYTHON)

  include(pybind11.cmake)

  pybind11_add_module(toke
    src/python/core.cpp
    src/python/train.h
    src/python/train.cpp
    src/python/exceptions.h
    src/python/exceptions.cpp
  )

  target_link_libraries(toke
    PUBLIC
      toke::core
      toke::train
  )

  install(TARGETS toke
    RUNTIME DESTINATION .
    ARCHIVE DESTINATION .
    LIBRARY DESTINATION .
  )

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
            toke::core
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
