@echo off

if not exist ..\build mkdir ..\build
pushd ..\build
cl /Zi /nologo ..\code\win32_handmade.cpp user32.lib
popd
