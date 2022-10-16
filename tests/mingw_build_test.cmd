@echo off
setlocal enableextensions
setlocal enabledelayedexpansion

rem set exe_name="../cmake_build/mtfind.exe"
set exe_name="../make_build/mtfind.exe"
rem set exe_name="../x64/Debug/mtfind.exe"
rem set exe_name="../x64/Release/mtfind.exe"
rem set exe_name="../Debug/mtfind.exe"
rem set exe_name="../Release/mtfind.exe"
set txt_folder=../resources

%exe_name%
echo errorlevel=%errorlevel%
echo.

%exe_name% %txt_folder%/test.bin "n?gger"
echo errorlevel=%errorlevel%
echo.

%exe_name% %txt_folder%/task.txt "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890+"
echo errorlevel=%errorlevel%
echo.

%exe_name% %txt_folder%/task.txt "?ad"
echo errorlevel=%errorlevel%
echo.

%exe_name% %txt_folder%/hugetext.bin "n?gger"
echo errorlevel=%errorlevel%
echo.

exit 0
