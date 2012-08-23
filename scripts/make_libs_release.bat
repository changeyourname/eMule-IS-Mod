@echo off
cd ..

@rem Prepare env
mkdir build\bin\Win32\Release
call "%VS100COMNTOOLS%\vsvars32.bat"

@rem Build OpenSSL
cd srclib\OpenSSL
set PATH=c:\perl\bin;d:\dev\nasm;%PATH%
perl Configure VC-WIN32 --prefix=.\bin
call ms\do_nasm.bat
nmake -f ms\ntdll.mak
copy /y /b out32dll\ssleay32.lib ..\..\build\bin\Win32\Release\
copy /y /b out32dll\libeay32.lib ..\..\build\bin\Win32\Release\
copy /y /b out32dll\ssleay32.dll ..\..\build\bin\Win32\Release\
copy /y /b out32dll\libeay32.dll ..\..\build\bin\Win32\Release\
copy /y /b out32dll\ssleay32.pdb ..\..\build\bin\Win32\Release\
copy /y /b out32dll\libeay32.pdb ..\..\build\bin\Win32\Release\
cd ../..

@rem Building other libraries
msbuild.exe /maxcpucount:2 /target:build /property:Configuration=Release,Platform=Win32 /fileLogger1 emule_3rdparty.sln

pause