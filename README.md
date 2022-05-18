# cppnat

**cppnat is a tcp hole punching tool helps you to expose you service on private network.**  


## Project

This project is written with Visual Studio Code.Project files are included as well.  
We can open project directly via Visual Studio Code.  
Third party requirements are listed on file ```thirdparty/requirement```.  
There are no project construction tools currently.You may need to install these 
lib youself.  
The network lib is using boost asio which is very fast in most circumstance.  

## Server

### description
Server is typically running on a server which has a public IP address.  
Server will wait for Client handshaking firstly,after handshaking is done,  
The rest connnections are considered users trying to connect the service  
on the private network which is the same network that client is using.  

### build
For release version choose `resease server ({os_type}) from Visual Studio Code 'Tasks:Run Build Tasks'`  
For debug version choose `build server ({os_type}) from Visual Studio Code 'Tasks:Run Build Tasks'`  


### config
`config.json` on the same cwd.
```
{
    "server": {
        "ip": "127.0.0.1",
        "port": 9544
    }
}
```
server.ip indicates which ip to listen on.  
server.port indicates which port to listen on.  


## client

### description
Client is usually dispatched on the server which only has private network.  
Client will retry connecting to server automatically if the connection to  
server is failed.  

### build
For release version choose `resease client ({os_type}) from Visual Studio Code 'Tasks:Run Build Tasks'`  
For debug version choose `build client ({os_type}) from Visual Studio Code 'Tasks:Run Build Tasks'`  


### config
`config.json` on the same cwd.
```
{
    "server": {
        "ip": "127.0.0.1",
        "port": 9544
    },
    "proxy": {
        "ip": "192.168.1.202",
        "port": 33123
    }
}
```
server.ip indicates the server ip.  
server.port indicates the server port.  
proxy.ip indicates the ip client should connect to.  
proxy.port indicates the port client should connect to.  