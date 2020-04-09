@echo off
glslc -c -fshader-stage=vertex vertex.glsl -o vertex.spv 
glslc -c -fshader-stage=fragment fragment.glsl -o fragment.spv 
echo.
pause