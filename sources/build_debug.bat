@echo off

mkdir ..\build
pushd ..\build
cl ..\sources\win32_handmade.cpp /Fe:win32_handmade.exe /Zi /Od /MDd /link User32.lib Gdi32.lib /DEBUG

popd