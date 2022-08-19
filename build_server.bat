@ECHO OFF

echo compiling bin\server.exe...

cl.exe ^
/EHsc ^
/nologo ^
/Zi ^
/MD ^
/Ox ^
/std:c++20 ^
/D_WIN32_WINNT=0x0601 ^
/Fe:bin\server.exe ^
src\message.cpp ^
src\proxy_socket.cpp ^
src\server.cpp ^
src\session.cpp ^
src\log.cpp ^
src\server\main.cpp ^
/Ithird_party\asio\asio\include ^
/Ithird_party\spdlog\include ^
/Ithird_party\jsonserializer\include 

if %ERRORLEVEL% EQU 0 echo compile bin\server.exe successfully!
pause