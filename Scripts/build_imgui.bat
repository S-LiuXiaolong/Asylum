@echo off
mkdir External\build\imgui
pushd External\build\imgui
cmake -S ..\..\src\imgui -B .
cmake --build . --config Release --target install
cmake --build . --config Debug --target install
popd