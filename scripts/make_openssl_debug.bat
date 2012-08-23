@echo off
call "%VS100COMNTOOLS%\vsvars32.bat"
cd ..\srclib\OpenSSL
set PATH=c:\perl\bin;d:\dev\nasm;%PATH%
perl Configure debug-VC-WIN32 --prefix=.\bin
call ms\do_nasm.bat
nmake -f ms\ntdll.mak

mkdir ..\..\build\bin\Win32\Debug
copy /y /b out32dll.dbg\ssleay32.lib ..\..\build\bin\Win32\Debug\
copy /y /b out32dll.dbg\libeay32.lib ..\..\build\bin\Win32\Debug\
copy /y /b out32dll.dbg\ssleay32.dll ..\..\build\bin\Win32\Debug\
copy /y /b out32dll.dbg\libeay32.dll ..\..\build\bin\Win32\Debug\
copy /y /b out32dll.dbg\ssleay32.pdb ..\..\build\bin\Win32\Debug\
copy /y /b out32dll.dbg\libeay32.pdb ..\..\build\bin\Win32\Debug\

pause