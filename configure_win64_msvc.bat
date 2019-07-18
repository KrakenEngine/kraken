rem call "%VSINSTALLDIR%VC\Auxiliary\Build\vcvars64.bat"
cmake -H. -G"Visual Studio 15 2017" -Bbuild -DCMAKE_TOOLCHAIN_FILE="toolchain/toolchain-x86_64-pc-windows-msvc.cmake"