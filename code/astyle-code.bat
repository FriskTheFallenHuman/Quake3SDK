@echo off
cls

astyle.exe -v --formatted --options=astyle-options.ini --exclude="renderer/glew-2.2.0" --exclude="renderer/stb" --exclude="renderer/tinyjpeg" --recursive *.h
astyle.exe -v --formatted --options=astyle-options.ini --exclude="renderer/glew-2.2.0" --exclude="renderer/stb" --exclude="renderer/tinyjpeg" --recursive *.cpp

pause
