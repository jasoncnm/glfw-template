@echo off


echo.
echo Compiling Shaders

pushd .\src\Shaders
call build.bat
popd

echo.
echo Shaders Compilation Complete

echo.
call cmake --build build
