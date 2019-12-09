cmake_minimum_required (VERSION 3.14)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(kraken_convert main.cpp)

if (WIN32)
  add_compile_definitions(UNICODE)
  # Enable Wildcard Globbing for command line arguments
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} setargv.obj")
else(WIN32)
  set(CMAKE_CXX_COMPILER "clang++")
endif(WIN32)

target_include_directories(kraken_convert PRIVATE ${CMAKE_SOURCE_DIR}/hydra/include ${CMAKE_SOURCE_DIR}/kraken/public) 

TARGET_LINK_LIBRARIES( kraken_convert kraken ${EXTRA_LIBS} )

set_target_properties( kraken_convert PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/output/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/output/bin
)
