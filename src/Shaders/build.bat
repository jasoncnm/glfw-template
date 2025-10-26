@echo off

IF NOT EXIST bytecode mkdir bytecode

%VULKAN_SDK%\Bin\glslc triangle.vert -o bytecode\vert.spv
%VULKAN_SDK%\Bin\glslc triangle.frag -o bytecode\frag.spv
