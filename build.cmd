@echo off

IF EXIST "out" RMDIR /S /Q "out"

MKDIR "out"
MKDIR "out\bin"

IF EXIST "config.xml" COPY "config.xml" "out\bin\config.xml"

cl.exe /EHsc /O2 /c "vstaudio.cpp" /Fo"out\vstaudio.obj"
rc.exe /fo"out\vstaudio.res" "vstaudio.rc"
link.exe /SUBSYSTEM:CONSOLE /RELEASE /OUT:"out\bin\vstaudio.exe" /DEBUG /PDB:"out\bin\vstaudio.pdb" "out\vstaudio.obj" "out\vstaudio.res" "ole32.lib" "oleaut32.lib" "user32.lib"
