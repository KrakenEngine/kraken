rem call "%VSINSTALLDIR%VC\Auxiliary\Build\vcvars64.bat"
cmake -H. -G Ninja -Bbuild -DCMAKE_TOOLCHAIN_FILE="tools/toolchain-x86_64-pc-windows-msvc.cmake"