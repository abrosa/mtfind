@echo off
pushd ..
pushd x64
pushd Debug
mtfind.exe ../../resources/test.bin "n?gger"
popd
popd
popd
pause && exit
rem mtfind.exe ../../resources/task.txt "?ad"
rem mtfind.exe ../../resources/Ulysses.txt "n?t?i?"
