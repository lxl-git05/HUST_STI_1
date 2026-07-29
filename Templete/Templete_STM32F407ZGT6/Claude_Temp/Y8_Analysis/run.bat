@echo off
chcp 65001 >nul
cd /d "%~dp0"
echo ========================================
echo   Y8 寻迹数据分析
echo ========================================
D:\conda_envs\claude_env\python.exe analyze.py %*
echo.
echo 分析完成，打开 analysis.png ...
start "" analysis.png
pause
