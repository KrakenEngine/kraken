cmake_minimum_required (VERSION 3.16)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(KRAKEN_CUBE_BUNDLE "${CMAKE_BINARY_DIR}/output/tests/kraken_cube.krbundle")
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/output/tests")

add_custom_command(
    OUTPUT ${KRAKEN_CUBE_BUNDLE}
    COMMAND kraken_convert -o ${KRAKEN_CUBE_BUNDLE} ${STANDARD_ASSET_BUNDLE}
    DEPENDS kraken_convert kraken_sdk standard_assets
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	COMMENT "Creating Cube Test Assets"
)

add_custom_target(kraken_cube_assets ALL
    DEPENDS ${KRAKEN_CUBE_BUNDLE}
    VERBATIM
)

if (WIN32)
  add_executable(kraken_cube WIN32 main_win.cpp harness.cpp harness.h hello_cube.cpp hello_cube.h)
  add_compile_definitions(UNICODE)
  list (APPEND EXTRA_LIBS shcore)

else(WIN32)
  add_executable(kraken_cube main_macos.mm harness.cpp harness.h hello_cube.cpp hello_cube.h)
  set(CMAKE_CXX_COMPILER "clang++")
  list (APPEND EXTRA_LIBS "-framework QuartzCore")
  list (APPEND EXTRA_LIBS "-framework Metal")
endif(WIN32)

add_dependencies(kraken_cube kraken_cube_assets)
target_include_directories(kraken_cube PRIVATE ${CMAKE_SOURCE_DIR}/hydra/include ${CMAKE_SOURCE_DIR}/kraken/public) 

TARGET_LINK_LIBRARIES( kraken_cube kraken ${EXTRA_LIBS} )

set_target_properties( kraken_cube PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/output/tests
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/output/tests
 
)
