@echo off

mkdir ..\build
pushd ..\build
:: O2 for optimization (Or Od for no optimisation), 
cl ..\sources\win32_handmade.cpp /Fe:win32_handmade.exe /Zi /Od /MDd /link User32.lib Gdi32.lib Xinput.lib Ole32.lib uuid.lib /DEBUG
:: /F1024 -> set the stack size to 1kB 

signtool sign /a /s My /n "HandmadeDev" /fd SHA256 win32_handmade.exe

popd