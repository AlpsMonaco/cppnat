@ECHO OFF

echo compiling bin\client.exe

cl.exe ^
/EHsc ^
/nologo ^
/Zi ^
/MD ^
/Ox ^
/std:c++20 ^
/D_WIN32_WINNT=0x0601 ^
/Fe:bin\client.exe ^
src\message.cpp ^
src\proxy_socket.cpp ^
src\client.cpp ^
src\session.cpp ^
src\log.cpp ^
src\client\main.cpp ^
/Ithird_party\asio\asio\include ^
/Ithird_party\spdlog\include ^
/Ithird_party\jsonserializer\include

if %ERRORLEVEL% EQU 0 echo compile bin\client.exe successfully!
pause