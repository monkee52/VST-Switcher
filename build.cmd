@ECHO OFF

SET "MOZDIR=C:\tmp\dev\mozilla-source\mozilla-central\js\src\build_OPT.OBJ\dist"
SET "V8DIR=C:\tmp\dev\v8\v8"

IF EXIST "out" RMDIR /S /Q "out"

MKDIR "out"
MKDIR "out\bin"

IF EXIST "config.xml" COPY "config.xml" "out\bin\config.xml"

SET COPTS=-DUNICODE -D_UNICODE
SET LOPTS=

SET "V8=0"
SET "SPIDERMONKEY=0"

IF EXIST "%V8DIR%" (
	IF "%SPIDERMONKEY%"=="0" (
		SET COPTS=%COPTS% -DUSE_V8 /I "%V8DIR%\include"
		SET LOPTS=%LOPTS% /LIBPATH:"%V8DIR%\..\v8lib" "v8.lib" "advapi32.lib" "winmm.lib" "dbghelp.lib" "shlwapi.lib"

		SET "V8=1"
	)
)

IF EXIST "%MOZDIR%" (
	IF "%V8%"=="0" (
		SET COPTS=%COPTS% -DUSE_SPIDERMONKEY /I "%MOZDIR%\include"
		SET LOPTS=%LOPTS% /LIBPATH:"%MOZDIR%\sdk\lib" "mozjs-51a1.lib"

		COPY "%MOZDIR%\bin\mozglue.dll" "out\bin\"
		COPY "%MOZDIR%\bin\mozjs-51a1.dll" "out\bin\"
		COPY "%MOZDIR%\bin\nspr4.dll" "out\bin\"
		COPY "%MOZDIR%\bin\plc4.dll" "out\bin\"
		COPY "%MOZDIR%\bin\plds4.dll" "out\bin\"

		SET "SPIDERMONKEY=1"
	)
)

cl.exe /EHsc /O2 /c %COPTS% "vstaudio.cpp" /Fo"out\vstaudio.obj"
rc.exe /fo"out\vstaudio.res" "vstaudio.rc"
link.exe /SUBSYSTEM:CONSOLE /RELEASE /OUT:"out\bin\vstaudio.exe" /DEBUG /PDB:"out\bin\vstaudio.pdb" "out\vstaudio.obj" "out\vstaudio.res" "ole32.lib" "oleaut32.lib" "user32.lib" %LOPTS%
