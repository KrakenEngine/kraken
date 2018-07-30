cmake_minimum_required (VERSION 2.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT WIN32)
  set(CMAKE_CXX_COMPILER "clang++")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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

IF(APPLE)
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

add_library(kraken SHARED ${SRCS} ${KRAKEN_PUBLIC_HEADERS})

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.54.0)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(kraken ${Boost_LIBRARIES})
endif()

# ---- Hydra ----
add_subdirectory(hydra)
include_directories(hydra/include)
target_link_libraries(kraken hydra)


# ---- OpenGL ----
find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS} )
target_link_libraries(kraken ${OPENGL_LIBRARIES})

# ---- GLAD ----
set(GLAD_EXTENSIONS "GL_EXT_texture_filter_anisotropic,GL_EXT_texture_compression_s3tc, GL_S3_s3tc" CACHE STRING "Gl exts" FORCE)
add_subdirectory(3rdparty/glad)
include_directories(${GLAD_INCLUDE_DIRS})
target_link_libraries(kraken glad)

# ---- GLFW ----
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory(3rdparty/glfw)
include_directories(3rdparty/glfw/include)
target_link_libraries(kraken glfw ${GLFW_LIBRARIES})

TARGET_LINK_LIBRARIES( kraken ${EXTRA_LIBS} )
SET_TARGET_PROPERTIES(
  kraken
PROPERTIES
  FRAMEWORK ON
  SOVERSION 0
  VERSION 0.1.0
  PUBLIC_HEADER "${KRAKEN_PUBLIC_HEADERS}"
  PRIVATE_HEADER "${PRIVATE_HEADER_FILES}"
  ARCHIVE_OUTPUT_DIRECTORY "archive"
  LIBRARY_OUTPUT_DIRECTORY "lib${LIB_SUFFIX}"
  OUTPUT_NAME kraken
)

# install(TARGETS kraken DESTINATION ${PROJECT_BINARY_DIR}/lib${LIB_SUFFIX})
# install(FILES ${KRAKEN_PUBLIC_HEADERS} DESTINATION ${PROJECT_BINARY_DIR}/include)

# INSTALL(TARGETS kraken 
#        LIBRARY DESTINATION "lib${LIB_SUFFIX}"
#        ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
#        PUBLIC_HEADER DESTINATION "include"
# )



# add_custom_target(package
#   COMMENT "Compressing..."
#   WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/archive"
#   COMMAND ${CMAKE_COMMAND} -E tar "cfvz" "kraken.tgz" "*"
# )
