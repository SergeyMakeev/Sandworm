CMake from https://cmake.org/ should be installed and placed to the system PATH.


1. Download LLVM and Clang source code from
   http://releases.llvm.org/download.html#5.0.0

   Direct links
	http://releases.llvm.org/5.0.0/llvm-5.0.0.src.tar.xz
	http://releases.llvm.org/5.0.0/cfe-5.0.0.src.tar.xz

2. Unpack "cfe-5.0.0.src.tar" to "llvm-5.0.0"

3. Unpack "cfe-5.0.0.src.tar" to "llvm-5.0.0/tools/clang"

4. The directory structure should look like this:

   llvm-5.0.0
   |
   +- (files from "llvm-5.0.0.src.tar")
   |
   +-tools
     |
     +-clang
       | 
       +- (files from "cfe-5.0.0.src.tar")

5. mkdir build
6. cd build
7. cmake ../llvm-5.0.0
8. Now open LLVM.sln and build ALL_BUILD project.
