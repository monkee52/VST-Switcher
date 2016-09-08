@echo off

SET "MOZDIR=C:\tmp\dev\mozilla-source\mozilla-central\js\src\build_OPT.OBJ\dist"

IF EXIST "out" RMDIR /S /Q "out"

MKDIR "out"
MKDIR "out\bin"

IF EXIST "config.xml" COPY "config.xml" "out\bin\config.xml"

IF EXIST "%MOZDIR%" (
	SET "COPTS=-DSPIDERMONKEY /I ""%MOZDIR%\include"""
	SET "LOPTS=/LIBPATH:""%MOZDIR%\sdk\lib"" ""mozjs-51a1.lib"""

	COPY "%MOZDIR%\bin\mozglue.dll" "out\bin\"
	COPY "%MOZDIR%\bin\mozjs-51a1.dll" "out\bin\"
	COPY "%MOZDIR%\bin\nspr4.dll" "out\bin\"
	COPY "%MOZDIR%\bin\plc4.dll" "out\bin\"
	COPY "%MOZDIR%\bin\plds4.dll" "out\bin\"
) ELSE (
	SET "COPTS="
	SET "LOPTS="
)

cl.exe /EHsc /O2 /c "vstaudio.cpp" /Fo"out\vstaudio.obj" %COPTS%
rc.exe /fo"out\vstaudio.res" "vstaudio.rc"
link.exe /SUBSYSTEM:CONSOLE /RELEASE /OUT:"out\bin\vstaudio.exe" /DEBUG /PDB:"out\bin\vstaudio.pdb" "out\vstaudio.obj" "out\vstaudio.res" "ole32.lib" "oleaut32.lib" "user32.lib" %LOPTS%
