set(triple x86_64-pc-windows-msvc)
set(CMAKE_C_COMPILER_TARGET "${LLVM_DEFAULT_TARGET_TRIPLE}" CACHE STRING "")
set(CMAKE_CXX_COMPILER_TARGET "${LLVM_DEFAULT_TARGET_TRIPLE}" CACHE STRING "")

set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER_TARGET ${triple})
set(CMAKE_C_FLAGS -m64)
set(CMAKE_CXX_FLAGS -m64)


# C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build>vcvars64.bat
