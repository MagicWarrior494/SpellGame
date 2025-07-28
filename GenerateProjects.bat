@echo off
echo ==========================================
echo Generating Visual Studio 2022 Project...
echo ==========================================

:: Run Premake bootstrap (fetch dependencies if needed)
premake5 --bootstrap

:: Generate Visual Studio 2022 solution
premake5 vs2022

echo.
echo ✅ Premake generation complete!
echo Open the generated SpellGame_Solution.sln file in Visual Studio.
pause