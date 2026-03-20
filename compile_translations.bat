@echo off
echo Compiling translations...

REM Find lrelease in Qt installation
set LRELEASE=lrelease

REM Compile French translation
%LRELEASE% translations\app_fr.ts -qm translations\app_fr.qm

if %ERRORLEVEL% EQU 0 (
    echo Translation compiled successfully!
    echo File created: translations\app_fr.qm
) else (
    echo Error: Could not compile translations.
    echo Make sure Qt bin directory is in your PATH
    echo Example: C:\Qt\6.7.3\mingw_64\bin
)

pause
