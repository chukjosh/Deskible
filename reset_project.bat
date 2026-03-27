@echo off
set PROJECT_DIR=C:\Users\chukjosh\Desktop\Projects\QtProjects\Deskible
echo Cleaning up build and user files for Deskible...

if exist "%PROJECT_DIR%\build" (
    echo Deleting build directory...
    rd /s /q "%PROJECT_DIR%\build"
)

if exist "%PROJECT_DIR%\CMakeLists.txt.user" (
    echo Deleting CMake user file...
    del "%PROJECT_DIR%\CMakeLists.txt.user"
)

if exist "%PROJECT_DIR%\Deskible.pro.user" (
    echo Deleting qmake user file...
    del "%PROJECT_DIR%\Deskible.pro.user"
)

echo.
echo Please follow these steps in Qt Creator:
echo 1. Open Qt Creator.
echo 2. Click 'File' - 'Open File or Project'.
echo 3. Select 'C:\Users\chukjosh\Desktop\Projects\QtProjects\Deskible\CMakeLists.txt'.
echo 4. Configure the project with the MinGW kit.
echo.
pause
