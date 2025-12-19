include_guard()

set(PYBIND11_FINDPYTHON NEW)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

FetchContent_Declare(pybind11
  URL "https://github.com/pybind/pybind11/archive/refs/tags/v3.0.1.zip"
  URL_HASH "SHA256=20fb420fe163d0657a262a8decb619b7c3101ea91db35f1a7227e67c426d4c7e"
)

FetchContent_MakeAvailable(pybind11)
