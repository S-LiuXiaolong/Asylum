@echo off

call .\Scripts\build_imgui.bat

cmake -S . -B build
PAUSE