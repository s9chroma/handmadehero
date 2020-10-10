@echo off

if not exist ..\build mkdir ..\build
pushd ..\build
cl /Zi /FC /nologo ..\code\win32_handmade.cpp user32.lib Gdi32.lib
popd
