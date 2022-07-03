rem call "%VSINSTALLDIR%VC\Auxiliary\Build\vcvars64.bat"
cmake -H. -G"Visual Studio 17 2022" -Bbuild -DCMAKE_TOOLCHAIN_FILE="toolchain/toolchain-x86_64-pc-windows-msvc.cmake"