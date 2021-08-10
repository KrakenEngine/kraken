cmake_minimum_required (VERSION 3.16)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32)
  add_executable(kraken_cube WIN32 main_win.cpp hello_cube.cpp)
  add_compile_definitions(UNICODE)
  set_property(TARGET kraken_cube PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "${STANDARD_ASSET_BUNDLE}")
else(WIN32)
  add_executable(kraken_cube main_macos.mm hello_cube.cpp)
  set(CMAKE_CXX_COMPILER "clang++")
endif(WIN32)

add_dependencies(kraken_cube standard_assets)
target_include_directories(kraken_cube PRIVATE ${CMAKE_SOURCE_DIR}/hydra/include ${CMAKE_SOURCE_DIR}/kraken/public) 

TARGET_LINK_LIBRARIES( kraken_cube kraken ${EXTRA_LIBS} )

# configure_file(${STANDARD_ASSET_BUNDLE} ${CMAKE_BINARY_DIR}/output/bin COPYONLY)

set_target_properties( kraken_cube PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/output/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/output/bin
)
