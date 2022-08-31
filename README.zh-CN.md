# cppnat

cppnat 是一个NAT内网穿透工具,用来帮助你暴露你的内网的服务到公网上。  

## 编译 (推荐)
1. 初始化 `thirt_party/` 下的git子模块。  
2. 使用`CMake`来生成项目文件。`CMakeLists.txt`在这个仓库的根目录中。  
### Linux
```sh
mkdir build ; cd build ; cmake .. ; make 
```
如果编译一切顺利，编译会输出 `server` 和 `client` 在这个仓库的 `bin/` 文件夹里。  
默认的 `server.json` 和 `client.json` 也在里面，修改他们来满足你的需要。  

### Windows
```bat
mkdir build & cd build && cmake .. 
```
打开 `cppnat.sln`，然后使用 `Visual Studio` 来编译 `cppnat` .  
编译成功的话，`Visual Studio` 会输出 `server.exe`和`client.exe`到`Release/`或者`Debug/`.里，取决于你使用什么模式编译。    
`server.json` 和 `client.json` 也会被拷贝过去修改他们来满足你的需要。

## 编译 (弃用)
1. 初始化 `thirt_party/` 下的git子模块。 
2. linux系统下执行 `build_server.sh` 和 `build_client.sh`  需要至少`g++ 10`.  
Windows需要`Visual Studio`,执行`build_server.bat` 和 `build_client.bat`.  
3. 如果编译顺利，编译会输出 `server` 和 `client` 在这个仓库的 `bin/` 文件夹里。  
默认的 `server.json` 和 `client.json` 也在里面，修改他们来满足你的需要。  

## 执行
`bin/server.json` 和 `bin/client.json` 模板配置文件在在这个仓库的 `bin/` 文件夹里。修改他们来满足你的需要。

## 配置

### 服务端
`bin/server` 一般部署在有公网的服务器上。每个TCP请求都会被重定向到cppnat客户端。

#### `server.json`
```json
{
    "bind_ip": "127.0.0.1",
    "bind_port": 54432
}
```
配置文件里配置公网服务器需要绑定的ip和端口  


### 客户端
`bin/client`一般部署在没有公网ip但能正常连接互联网的服务器上，通常是经过了网络地址转换。


#### `client.json`
```json
{
    "server_ip": "127.0.0.1",
    "server_port": 54432,
    "proxy_ip": "127.0.0.1",
    "proxy_port": 33123
}
```
配置 `server_ip` 和 `server_port` 来指定cppnat的ip和端口
配置 `proxy_ip` 和 `proxy_port` 来指定需要代理的内网TCP服务

## 项目

该项目由`Visual Studio Code`编写，可以用`Visual Studio Code`直接打开。  