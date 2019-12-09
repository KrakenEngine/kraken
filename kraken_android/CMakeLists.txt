set(KRAKEN_PUBLIC_HEADERS "${KRAKEN_PUBLIC_HEADERS}" PARENT_SCOPE)

# Private Implementation
add_sources(vulkan_wrapper.cpp)
include_directories(.)
