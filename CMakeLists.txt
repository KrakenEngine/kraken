cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32)
  add_definitions(-D_HAS_EXCEPTIONS=0)
endif()

if (NOT WIN32 AND NOT ANDROID)
  set(CMAKE_CXX_COMPILER "clang++")
endif()

project (Kraken)

macro (add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else()
            list (APPEND SRCS "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate SRCS to parent directory
        set (SRCS ${SRCS} PARENT_SCOPE)
    endif()
endmacro()

macro (add_standard_asset)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND KRAKEN_STANDARD_ASSETS "${_relPath}/${_src}")
        else()
            list (APPEND KRAKEN_STANDARD_ASSETS "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate KRAKEN_STANDARD_ASSETS to parent directory
        set (KRAKEN_STANDARD_ASSETS ${KRAKEN_STANDARD_ASSETS} PARENT_SCOPE)
    endif()
endmacro()

macro (add_public_header)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND KRAKEN_PUBLIC_HEADERS "${_relPath}/${_src}")
        else()
            list (APPEND KRAKEN_PUBLIC_HEADERS "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate KRAKEN_PUBLIC_HEADERS to parent directory
        set (KRAKEN_PUBLIC_HEADERS ${KRAKEN_PUBLIC_HEADERS} PARENT_SCOPE)
    endif()
endmacro()

macro (add_private_headers)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND KRAKEN_PRIVATE_HEADERS "${_relPath}/${_src}")
        else()
            list (APPEND KRAKEN_PRIVATE_HEADERS "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate KRAKEN_PRIVATE_HEADERS to parent directory
        set (KRAKEN_PRIVATE_HEADERS ${KRAKEN_PRIVATE_HEADERS} PARENT_SCOPE)
    endif()
endmacro()

macro (add_source_and_header)
    foreach (_src ${ARGN})
        add_sources("${_src}.cpp")
        add_private_headers("${_src}.h")
    endforeach()
endmacro()

IF(APPLE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations -Wno-c++11-extensions")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated -Wno-deprecated-declarations -Wno-c++11-extensions")
   # SET(GUI_TYPE MACOSX_BUNDLE)
   # INCLUDE_DIRECTORIES ( /Developer/Headers/FlatCarbon )
   FIND_LIBRARY(APPKIT_LIBRARY AppKit)
   FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
   FIND_LIBRARY(AUDIO_TOOLBOX_LIBRARY AudioToolbox)
   FIND_LIBRARY(ACCELERATE_LIBRARY Accelerate)
   MARK_AS_ADVANCED (APPKIT_LIBRARY
                     OPENGL_LIBRARY
                     AUDIO_TOOLBOX_LIBRARY
                     ACCELERATE_LIBRARY)
   SET(EXTRA_LIBS ${APPKIT_LIBRARY} ${OPENGL_LIBRARY} ${AUDIO_TOOLBOX_LIBRARY} ${ACCELERATE_LIBRARY})
   FIND_PATH(COCOA_INCLUDE_DIR OpenGL/gl3.h)
ENDIF (APPLE)

add_subdirectory(kraken)

add_public_header(hydra/include/aabb.h)
add_public_header(hydra/include/hitinfo.h)
add_public_header(hydra/include/hydra.h)
add_public_header(hydra/include/matrix2.h)
add_public_header(hydra/include/matrix2x3.h)
add_public_header(hydra/include/matrix4.h)
add_public_header(hydra/include/quaternion.h)
add_public_header(hydra/include/scalar.h)
add_public_header(hydra/include/triangle3.h)
add_public_header(hydra/include/vector2.h)
add_public_header(hydra/include/vector3.h)
add_public_header(hydra/include/vector4.h)
add_public_header(hydra/include/vector2i.h)

add_public_header(mimir/include/mimir.h)
add_public_header(mimir/include/block.h)

add_public_header(siren/include/siren.h)
add_public_header(siren/include/dsp.h)

# ---- Android ----
if(ANDROID)
add_subdirectory(kraken_android)
endif()

# ---- Hydra ----
add_subdirectory(hydra)
include_directories(hydra/include)
list (APPEND EXTRA_LIBS hydra)

# ---- Siren ----
add_subdirectory(siren)
include_directories(siren/include)
list (APPEND EXTRA_LIBS siren)

# ---- Mimir ----
add_subdirectory(mimir)
include_directories(mimir/include)
list (APPEND EXTRA_LIBS mimir)

# ---- Compressonator CMP_Core ----
add_subdirectory(3rdparty/compressonator/cmp_core)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/compressonator/cmp_core/source")
list (APPEND EXTRA_LIBS CMP_Core)

# ---- Vulkan ----
add_library(vulkan INTERFACE)
set(VULKAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/vulkan/include)
target_sources(vulkan INTERFACE ${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h)
target_include_directories(vulkan INTERFACE ${VULKAN_INCLUDE_DIR})

target_compile_definitions(vulkan INTERFACE VK_NO_PROTOTYPES)

if(ANDROID)
    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
elseif(WIN32)
    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX)
    # See whether X11 is available. If not, fall back to direct-to-display mode.
    find_package(X11 QUIET)
    if (X11_FOUND)
        target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XCB_KHR)
    else()
        set(DIRECT_TO_DISPLAY TRUE)
        set(DIRECT_TO_DISPLAY TRUE PARENT_SCOPE)
        target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_DISPLAY_KHR)
    endif()
endif()

list (APPEND EXTRA_LIBS vulkan)

# ---- Vulkan Memory Allocator ----
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/vma/include")

# ---- Volk ----
# volk
set(VOLK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/volk")
set(VOLK_FILES
    "${VOLK_DIR}/volk.c"
    "${VOLK_DIR}/volk.h")

add_library(volk STATIC ${VOLK_FILES})

target_link_libraries(volk PUBLIC vulkan)

target_include_directories(volk PUBLIC ${VOLK_DIR})

set_property(TARGET volk PROPERTY FOLDER "3rdparty")

list (APPEND EXTRA_LIBS volk)

# ---- GLSlang ----
if(NOT ANDROID)
set(ENABLE_GLSLANG_BINARIES OFF)
set(ENABLE_GLSLANG_INSTALL OFF)
set(ENABLE_SPVREMAPPER OFF)
add_subdirectory(3rdparty/glslang)
include_directories(3rdparty/glslang/Public)
list (APPEND EXTRA_LIBS "glslang")
list (APPEND EXTRA_LIBS "SPIRV")
endif()

# ---- SPIRV-Reflect ----
include_directories(3rdparty/spirv-reflect)
add_sources(3rdparty/spirv-reflect/spirv_reflect.c)

source_group("Public Headers", FILES ${KRAKEN_PUBLIC_HEADERS})
source_group("Private Headers", FILES ${KRAKEN_PRIVATE_HEADERS})

add_library(kraken STATIC ${SRCS} ${KRAKEN_PUBLIC_HEADERS} ${KRAKEN_PRIVATE_HEADERS})
TARGET_LINK_LIBRARIES( kraken ${EXTRA_LIBS} )
SET_TARGET_PROPERTIES(
  kraken
PROPERTIES
  PUBLIC_HEADER "${KRAKEN_PUBLIC_HEADERS}"
  PRIVATE_HEADER "${KRAKEN_PRIVATE_HEADERS}"
  ARCHIVE_OUTPUT_DIRECTORY "lib${LIB_SUFFIX}"
  OUTPUT_NAME kraken_static
)

add_library(kraken_dynamic SHARED ${SRCS} ${KRAKEN_PUBLIC_HEADERS} ${KRAKEN_PRIVATE_HEADERS})
TARGET_LINK_LIBRARIES( kraken_dynamic ${EXTRA_LIBS} )
SET_TARGET_PROPERTIES(
  kraken_dynamic
PROPERTIES
  PUBLIC_HEADER "${KRAKEN_PUBLIC_HEADERS}"
  PRIVATE_HEADER "${KRAKEN_PRIVATE_HEADERS}"
  ARCHIVE_OUTPUT_DIRECTORY "lib${LIB_SUFFIX}"
  OUTPUT_NAME kraken
)

install(TARGETS kraken
        RUNTIME DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/export/${CMAKE_BUILD_TYPE}/bin/win
        PUBLIC_HEADER DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/export/include
        ARCHIVE DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/export/${CMAKE_BUILD_TYPE}/lib/win
        LIBRARY DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/export/${CMAKE_BUILD_TYPE}/lib/win
)

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
  install (FILES $<TARGET_PDB_FILE:kraken>
           DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/export/lib/win)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

add_subdirectory(standard_assets)


SET(STANDARD_ASSET_LIST_FILE "${CMAKE_BINARY_DIR}/standard_assets_list")
SET(STANDARD_ASSET_BUNDLE "${CMAKE_BINARY_DIR}/output/assets/standard_assets.krbundle")

SET(STANDARD_ASSET_LIST_FILE_CONTENTS "")
FOREACH(line ${KRAKEN_STANDARD_ASSETS})
   SET(STANDARD_ASSET_LIST_FILE_CONTENTS "${STANDARD_ASSET_LIST_FILE_CONTENTS}${line}\n")
ENDFOREACH(line)
FILE(WRITE ${STANDARD_ASSET_LIST_FILE} ${STANDARD_ASSET_LIST_FILE_CONTENTS})
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/output/assets")

add_custom_command(
    OUTPUT ${STANDARD_ASSET_BUNDLE}
    COMMAND kraken_convert -c -i ${STANDARD_ASSET_LIST_FILE} -o ${STANDARD_ASSET_BUNDLE}
    DEPENDS kraken_convert ${KRAKEN_STANDARD_ASSETS} ${STANDARD_ASSET_LIST_FILE}
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	COMMENT "Creating Standard Assets"
)

add_custom_target(standard_assets ALL
    DEPENDS ${STANDARD_ASSET_BUNDLE}
    SOURCES ${KRAKEN_STANDARD_ASSETS}
    VERBATIM
)

foreach(header_file ${KRAKEN_PUBLIC_HEADERS})
  get_filename_component(header_file_name "${header_file}" NAME)
  list(APPEND KRAKEN_PUBLIC_HEADERS_OUTPUT "${header_file_name}")
endforeach()
list(TRANSFORM KRAKEN_PUBLIC_HEADERS_OUTPUT PREPEND "${CMAKE_BINARY_DIR}/output/include/")

file(COPY ${KRAKEN_PUBLIC_HEADERS} DESTINATION "${CMAKE_BINARY_DIR}/output/include")

add_custom_target(kraken_sdk ALL
    DEPENDS ${STANDARD_ASSET_BUNDLE} ${KRAKEN_PUBLIC_HEADERS_OUTPUT}
    VERBATIM
)

add_custom_command(
    TARGET kraken_sdk
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:kraken>
        ${CMAKE_BINARY_DIR}/output/lib/$<TARGET_FILE_NAME:kraken>
)

add_custom_command(
    TARGET kraken_sdk
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:kraken_dynamic>
        ${CMAKE_BINARY_DIR}/output/lib/$<TARGET_FILE_NAME:kraken_dynamic>
)

add_subdirectory(tests)
add_subdirectory(tools)

set_target_properties( kraken PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/output/lib
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/output/lib
)
