rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../x86_64-mingw.cmake -G "MinGW Makefiles" ..
cmake --build .