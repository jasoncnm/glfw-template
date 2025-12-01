@echo off

IF NOT EXIST bytecode mkdir bytecode

FOR %%f IN (*.vert) DO %VULKAN_SDK%\Bin\glslc %%f -o bytecode\%%~nf_vert.spv
FOR %%f IN (*.frag) DO %VULKAN_SDK%\Bin\glslc %%f -o bytecode\%%~nf_frag.spv

REM %VULKAN_SDK%\Bin\glslc triangle.vert -o bytecode\vert.spv
REM %VULKAN_SDK%\Bin\glslc triangle.frag -o bytecode\frag.spv
