# Fast DDS Cross Compile for ARM

## 安装交叉编译工具

这里下载[arm-v7a-linux-gnueabihf](https://aur.archlinux.org/packages/?O=0&SeB=nd&K=arm-v7a-linux-gnueabihf&outdated=&SB=n&SO=a&PP=50&do_Search=Go)

## Fast-DDS 依赖

### Asio

下载 [Asio 1.16.1](https://think-async.com/Asio/Download.html)

由于asio c++ libary, 只有头文件,所以不需要编译,只需要拷贝header到交叉编译环境下的include文件夹就可以

```shell
cp -R ~/asio-1.16.1/include/asio /opt/OSELAS.Toolchain-2016.06.0/arm-v7a-linux-gnueabihf/gcc-5.4.0-glibc-2.23-binutils-2.26-kernel-4.6-sanitized/sysroot-arm-v7a-linux-gnueabihf/usr/local/include/
cp ~/asio-1.16.1/include/asio.hpp /opt/OSELAS.Toolchain-2016.06.0/arm-v7a-linux-gnueabihf/gcc-5.4.0-glibc-2.23-binutils-2.26-kernel-4.6-sanitized/sysroot-arm-v7a-linux-gnueabihf/usr/local/include/
```

### TinyXML2

下载 [TinyXML2](https://github.com/leethomason/tinyxml2)

修改`CMakeLists.txt`, 添加如下设置

```cmake

message("=== set sysroot ===")

set(CMAKE_SYSROOT "/opt/OSELAS.Toolchain-2016.06.0/arm-v7a-linux-gnueabihf/gcc-5.4.0-glibc-2.23-binutils-2.26-kernel-4.6-sanitized/sysroot-arm-v7a-linux-gnueabihf")
set(CMAKE_INSTALL_PREFIX "${CMAKE_SYSROOT}")

set(CMAKE_C_COMPILER /opt/OSELAS.Toolchain-2016.06.0/arm-v7a-linux-gnueabihf/gcc-5.4.0-glibc-2.23-binutils-2.26-kernel-4.6-sanitized/bin/arm-v7a-linux-gnueabihf-gcc)
set(CMAKECXXCOMPILER /opt/OSELAS.Toolchain-2016.06.0/arm-v7a-linux-gnueabihf/gcc-5.4.0-glibc-2.23-binutils-2.26-kernel-4.6-sanitized/bin/arm-v7a-linux-gnueabihf-g++)
set(CMAKE_LINKER /opt/OSELAS.Toolchain-2016.06.0/arm-v7a-linux-gnueabihf/gcc-5.4.0-glibc-2.23-binutils-2.26-kernel-4.6-sanitized/bin/arm-v7a-linux-gnueabihf-ld)


set(CMAKE_FIND_ROOT_PATH /opt/OSELAS.Toolchain-2016.06.0/arm-v7a-linux-gnueabihf/gcc-5.4.0-glibc-2.23-binutils-2.26-kernel-4.6-sanitized/sysroot-arm-v7a-linux-gnueabihf)
include_directories("${CMAKE_FIND_ROOT_PATH}/usr/include/")
include_directories("${CMAKE_FIND_ROOT_PATH}/usr/local/include/")
include_directories("${CMAKE_FIND_ROOT_PATH}/include/")
message("--- add include ---")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

编译安装到交叉编译环境下:

```shell
cmake ..  -DCMAKE_INSTALL_PREFIX=/opt/OSELAS.Toolchain-2016.06.0/arm-v7a-linux-gnueabihf/gcc-5.4.0-glibc-2.23-binutils-2.26-kernel-4.6-sanitized/sysroot-arm-v7a-linux-gnueabihf

sudo cmake --build . --target install
```

### Foonathan memory

```shell
git clone https://github.com/eProsima/foonathan_memory_vendor.git
cd foonathan_memory_vendor
```
修改`CMakeLists.txt`, 添加和`TinyXML2`中的类似设置

```shell
mkdir foonathan_memory_vendor/build
cd foonathan_memory_vendor/build
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/OSELAS.Toolchain-2016.06.0/arm-v7a-linux-gnueabihf/gcc-5.4.0-glibc-2.23-binutils-2.26-kernel-4.6-sanitized/sysroot-arm-v7a-linux-gnueabihf -DBUILD_SHARED_LIBS=ON
sudo cmake --build . --target install
```

### Fast CDR

```shell
git clone https://github.com/eProsima/Fast-CDR.git
cd Fast-CDR
```

修改`CMakeLists.txt`, 添加和`TinyXML2`中的类似设置

```shell
mkdir Fast-CDR/build
cd Fast-CDR/build
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/OSELAS.Toolchain-2016.06.0/arm-v7a-linux-gnueabihf/gcc-5.4.0-glibc-2.23-binutils-2.26-kernel-4.6-sanitized/sysroot-arm-v7a-linux-gnueabihf
sudo cmake --build . --target install
```

## Fast-DDS

```shell
git clone https://github.com/eProsima/Fast-DDS.git
cd Fast-DDS
```

修改`CMakeLists.txt`, 添加和`TinyXML2`中的类似设置

```shell
mkdir Fast-DDS/build
cd Fast-DDS/build
cmake ..  -DCMAKE_INSTALL_PREFIX=~/Fast-DDS/install -DCMAKE_PREFIX_PATH=~/Fast-DDS/install
sudo cmake --build . --target install
```

## Compile Fast DDS Example

Refer to code [Fast DDS Example](./Fast_DDS_Example.md), 同样修改`CMakeLists.txt` 添加交叉编译的参数

```shell
cd dds_helloworld/build
rm -rf *
cmake ..
make clean && make
```
## Reference

[Install the ARM cross compiler toolchain on your Linux Ubuntu PC](https://www.acmesystems.it/arm9_toolchain)
[交叉编译器 arm-linux-gnueabi 和 arm-linux-gnueabihf 的区别](https://www.cnblogs.com/xiaotlili/p/3306100.html)