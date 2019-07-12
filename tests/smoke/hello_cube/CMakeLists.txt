cmake_minimum_required (VERSION 3.14)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32)
  add_executable(kraken_cube WIN32 main_win.cpp)
  add_compile_definitions(UNICODE)
else(WIN32)
  add_executable(kraken_cube main_macos.mm)
  set(CMAKE_CXX_COMPILER "clang++")
endif(WIN32)

target_include_directories(kraken_cube PRIVATE ${CMAKE_SOURCE_DIR}/hydra/include ${CMAKE_SOURCE_DIR}/kraken/public) 

TARGET_LINK_LIBRARIES( kraken_cube kraken ${EXTRA_LIBS} )

