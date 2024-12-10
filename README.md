# libcplus



## Require
1. cmake




## How to build
1. cd libcplus
2. mkdir build
3. cd build
4. cmake ..
5. make


## APPENDIX
### How to install cmake
1. wget https://github.com/Kitware/CMake/releases/download/v3.31.1/cmake-3.31.1.tar.gz
2. tar -zxvf cmake-3.31.1.tar.gz
3. cd cmake-3.31.1
4. ./bootstrap
5. make -j4
6. sudo make install

# TroubleShooting
Q1: CMake not able to find OpenSSL library
ans: sudo apt-get install libssl-dev

Q2: The C++ compiler does not support C++11 (e.g. std::unique_ptr) building cmake
ans: I copied the CMake source code to my system drive. If you make CMake on a remote harddrive it will fail and error just like above.