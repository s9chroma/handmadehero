@echo off

if not exist ..\build mkdir ..\build
pushd ..\build
cl /Zi /nologo ..\code\win32_handmade.cpp User32.lib Gdi32.lib
popd
