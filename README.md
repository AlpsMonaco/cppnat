# cppnat

writen in cpp,cppnat is a NAT reverse proxy tool (aslo known as a hole punching tool) helps you expose your private IP and port and bind them onto a server with public IP,access your private service anywhere.

## build (recommended)
1. Init git submodules in `thirt_party/`.  
2. Use CMake to generate platform project file.`CmakeLists.txt` file is in the root path of this repository.  
### Linux
```sh
mkdir build ; cd build ; cmake .. ; make 
```
If the compile is successful,you will have both `server` and `client` in the `bin/` directory of this repository.  
The default `server.json` and `client.json` are also included in the `bin/` directory,change them to satisfy you needs.

### Windows
```bat
mkdir build & cd build && cmake .. 
```
Open `cppnat.sln` and compile `cppnat` using `Visual Studio`.  
You will have `server.exe` and `client.exe` in `Visual Studio` runtime output directory,could be `Release/` or `Debug/`.  
Also `server.json` and `client.json` will be copied to the directory,  
change them to satisfy you needs.


## build (deprecated)
1. Init git submodules in `thirt_party/`.
2. run `build_server.sh` and `build_client.sh` on linux os, requires at least `g++ 10`.  
For Windows system with MSVC compiler `cl.exe` has installed,run `build_server.bat` and `build_client.bat`.
3. If the compile is successful,you will have `server` and `client` in the `bin/` directory.

## run
There are `bin/server.json` and `bin/client.json` template config file in the `bin/` directory.Simply change them to satisfy you needs.

## config

### server
`bin/server` is typically dispatched on a server with public ip.Every TCP connection to the ip and the port you have configured will redirect to nat client.  

#### `server.json`
```json
{
    "bind_ip": "127.0.0.1",
    "bind_port": 54432
}
```
There are only two keys in the config json file indicates on which ip and which port the nat server should bind.  


### client
`bin/client` is typically dispatched on a computer without a public ip but able to connect to the Internet with NAT.A client helps you expose you service 
such as ssh or rdp in the computer to the Internet that you could access it anywhere.  

#### `client.json`
```json
{
    "server_ip": "127.0.0.1",
    "server_port": 54432,
    "proxy_ip": "127.0.0.1",
    "proxy_port": 33123
}
```
configure `server_ip` and `server_port` to a nat server's binded ip and binded port.  
configure `proxy_ip` and `proxy_port` to you local private service.

**Now you could access you private service anywhere**
based on asio,this library performs well under high concurrency as well.  


## Project

This library is written with `Visual Studio Code`,you could open this project of root path and launch it directly.