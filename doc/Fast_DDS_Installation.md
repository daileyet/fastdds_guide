# Fast DDS Installation

## Linux installation from binaries

一般在虚拟机上的Ubuntu系统最快捷的方式是从binaries安装.

从官方[下载地址](https://www.eprosima.com/index.php/component/ars/repository/eprosima-fast-dds)下载[最新版本2.1.0](https://www.eprosima.com/index.php/component/ars/repository/eprosima-fast-dds/eprosima-fast-dds-2-1-0/eprosima_fast-dds-2-1-0-linux-tgz?format=raw)

### 解压

```shell
mkdir -p ~/workspace/fast_dds && tar -xvf ~/workspace/eProsima_Fast-DDS-2.1.0-Linux.tgz -C ~/worksapce/fast_dds
```

### Precondtion

```shell
sudo apt install libasio-dev libtinyxml2-dev
sudo apt install g++
```

#### CMake

CMake安装最新版, 通过`sudo apt install cmake`安装的cmake版本可能比较早, 建议卸载后安装最新版本的cmake

```shell
sudo apt remove cmake
sudo rm -rf /usr/bin/cmake

```

```shell
cd ~/workspace
wget https://github.com/Kitware/CMake/releases/download/v3.19.2/cmake-3.19.2-Linux-x86_64.sh
chmod a+x cmake-3.19.2-Linux-x86_64.sh
sudo ./cmake-3.19.2-Linux-x86_64.sh
```

after that, installation folder `cmake-3.19.2-Linux-x86_64` will created under `~/workspace`.

```
nano ~/.bashrc
# add follow statement at the end

export PATH=$PATH:/pathto/cmake-3.19.2-Linux-x86_64/bin

```

```
source ~/.bashrc
cmake -version
# it will show cmake version 3.19.2
```

### 安装

```shell
cd ~/workspace/fast_dds && sudo ./install.sh
```

```shell
# Add it permanently to the PATH by executing
echo 'export LD_LIBRARY_PATH=/usr/local/lib/' >> ~/.bashrc
```

## Linux installation from source

参考 https://fast-dds.docs.eprosima.com/en/latest/installation/sources/sources_linux.html

### Fast DDS-Gen installation

其中Fast DDS-Gen安装需要JDK和gradle构建工具, 这里列出gradle安装需要注意事项:

```shell
cd ~/worksapce
wget https://services.gradle.org/distributions/gradle-6.7.1-bin.zip
unzip gradle-6.7.1-bin.zip

echo `export PATH=$PATH:/pathto/worksapce/gradle-6.7.1/bin` >> ~/.bashrc
source ~/.bashrc

gradle -v
# show Gradle 6.7.1
```

configure gradle proxy
```shell
# create file ~/.gradle/gradle.properties if not exist
touch ~/.gradle/gradle.properties
```
add http proxy setting as below:

```ini
systemProp.http.proxyHost=127.0.0.1
systemProp.http.proxyPort=3128
systemProp.http.proxyUser=
systemProp.http.proxyPassword=
systemProp.http.nonProxyHosts=localhost

systemProp.https.proxyHost=127.0.0.1
systemProp.https.proxyPort=3128
systemProp.https.proxyUser=
systemProp.https.proxyPassword=
systemProp.https.nonProxyHosts=localhost

```