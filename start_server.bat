@echo off
cd /d "%~dp0server"
call venv\Scripts\activate.bat
python app.py
pause
