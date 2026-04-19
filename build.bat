@echo off
echo Building HammerDown project...
echo.

REM Set Qt paths (adjust if Qt is installed elsewhere)
set QT_PATH=C:\Qt\6.7.3\mingw_64
set "PATH=%QT_PATH%\bin;%PATH%"

REM Check if Qt tools are available
where qmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo WARNING: qmake not found in PATH. Trying to set Qt path...
    set "PATH=%QT_PATH%\bin;%PATH%"
    
    where qmake >nul 2>nul
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: qmake still not found. Please check Qt installation.
        echo Expected Qt installation: %QT_PATH%
        pause
        exit /b 1
    )
)

REM Create build directory
if not exist "build" mkdir build
cd build

echo Running qmake with Qt SQL module...
qmake ../HammerDown.pro CONFIG+=release
if %ERRORLEVEL% NEQ 0 (
    echo WARNING: qmake failed, trying CMake...
    
    REM Try CMake as fallback
    where cmake >nul 2>nul
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: cmake not found either. Please install Qt with CMake.
        pause
        exit /b 1
    )
    
    echo Running CMake...
    cmake .. -DCMAKE_PREFIX_PATH=%QT_PATH% -DCMAKE_BUILD_TYPE=Release
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: CMake configuration failed
        pause
        exit /b 1
    )
    
    echo Building with CMake...
    cmake --build . --config Release
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: CMake build failed
        pause
        exit /b 1
    )
) else (
    echo Building project with MinGW...
    mingw32-make
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Build failed
        pause
        exit /b 1
    )
)

echo.
echo Build completed successfully!
echo Executable should be located at: build\HammerDown.exe
echo.
echo If SQL errors persist, ensure Qt SQL module is installed:
echo - Check: %QT_PATH%\plugins\sqldrivers\ folder
echo - Should contain: qsqlodbc.dll, qsqloci.dll (for Oracle)
pause
