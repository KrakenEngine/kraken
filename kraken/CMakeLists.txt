include_directories(public)
add_subdirectory(public)
set(KRAKEN_PUBLIC_HEADERS "${KRAKEN_PUBLIC_HEADERS}" PARENT_SCOPE)

add_sources(scalar.cpp)
add_sources(vector2.cpp)
add_sources(vector3.cpp)
add_sources(vector4.cpp)
add_sources(triangle3.cpp)
add_sources(quaternion.cpp)
add_sources(matrix4.cpp)
add_sources(aabb.cpp)
add_sources(hitinfo.cpp)

# Private Implementation
add_sources(KRDataBlock.cpp)
