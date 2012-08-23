@echo off 
echo Cleaning all!!!
cd ..

echo Removing files: *.ncb *.suo *.name etc.
del /S /F /Q /A:H *.suo
del /S /F /Q *.sdf
del /S /F /Q *.log
del /S /F /Q *vcxproj*.user
del /S /F /Q *vcproj*.user
del /S /F /Q *pro.user
del /S /F /Q *.aps
del /S /F /Q *.ncb

echo Removing folders: build ipch
rmdir /S /Q build
rmdir /S /Q ipch

echo Cleaning xCatalog
rmdir /S /Q xcatalog\GeneratedFiles
del /S /F /Q xcatalog\ui_*.h

echo Cleaning OpenSSL
rmdir /S /Q srclib\OpenSSL\out32dll
rmdir /S /Q srclib\OpenSSL\out32dll.dbg
rmdir /S /Q srclib\OpenSSL\tmp32dll
rmdir /S /Q srclib\OpenSSL\tmp32dll.dbg
rmdir /S /Q xcatalog_app-build-desktop

echo Complete!
pause