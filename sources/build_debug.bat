@echo off

mkdir ..\build
pushd ..\build
cl ..\sources\win32_handmade.cpp /Fe:win32_handmade.exe /Zi /Od /MDd /link User32.lib Gdi32.lib Xinput.lib Ole32.lib uuid.lib /DEBUG
:: /F1024 -> set the stack size to 1kB 
popd