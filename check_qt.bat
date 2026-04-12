@echo off
echo Checking Qt installation and SQL module availability...
echo.

REM Set Qt paths
set QT_PATH=C:\Qt\6.7.3\mingw_64
set PATH=%QT_PATH%\bin;%PATH%

echo Qt Path: %QT_PATH%
echo.

echo Checking Qt installation...
where qmake >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo ✓ qmake found
    qmake -version
) else (
    echo ✗ qmake NOT found
)

echo.
echo Checking Qt SQL module...
if exist "%QT_PATH%\plugins\sqldrivers\qsqlodbc.dll" (
    echo ✓ QODBC driver found
) else (
    echo ✗ QODBC driver NOT found
)

if exist "%QT_PATH%\plugins\sqldrivers\qsqloci.dll" (
    echo ✓ QOCI driver found
) else (
    echo ✗ QOCI driver NOT found
)

if exist "%QT_PATH%\plugins\sqldrivers\qsqlite.dll" (
    echo ✓ QSQLite driver found
) else (
    echo ✗ QSQLite driver NOT found
)

echo.
echo Checking Qt SQL headers...
if exist "%QT_PATH%\include\QtSql\QSqlDatabase" (
    echo ✓ QSqlDatabase header found
) else (
    echo ✗ QSqlDatabase header NOT found
)

echo.
echo Available SQL drivers:
echo Creating test program to check drivers...

(
echo #include ^<QCoreApplication^>
echo #include ^<QSqlDatabase^>
echo #include ^<QDebug^>
echo int main^(int argc, char *argv[]^) {
echo     QCoreApplication app^(argc, argv^);
echo     qDebug^(^)^<^< "Available SQL drivers:" ^<^< QSqlDatabase::drivers^(^);
echo     return 0;
echo }
) > test_sql.cpp

echo Compiling test program...
cd /d "%TEMP%"
g++ -I"%QT_PATH%\include" -I"%QT_PATH%\include\QtCore" -I"%QT_PATH%\include\QtSql" -L"%QT_PATH%\lib" -lQt6Sql -lQt6Core test_sql.cpp -o test_sql.exe 2>nul

if exist test_sql.exe (
    echo ✓ Test compilation successful
    echo Running test...
    test_sql.exe
    del test_sql.exe test_sql.cpp
) else (
    echo ✗ Test compilation failed
)

cd /d "h:\7aw"
echo.
echo Diagnostic complete.
pause
