@echo off

IF EXIST "out" RMDIR /S /Q "out"

MKDIR "out"
MKDIR "out\bin"

cl.exe /c "vstaudio.cpp" /Fo"out\vstaudio.obj"
rc.exe /fo"out\vstaudio.res" "vstaudio.rc"
link.exe /SUBSYSTEM:CONSOLE /RELEASE /OUT:"out\bin\vstaudio.exe" /DEBUG /PDB:"out\vstaudio.pdb" "out\vstaudio.obj" "out\vstaudio.res"
