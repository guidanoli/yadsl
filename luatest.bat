@echo off
SET LUA_CPATH=%cd%\lib64\?.dll
FOR /r %%i IN (src\*_test.lua) DO @%LUA% %%i
ECHO ~~~~~~~~~~~~~~~~~~~~~~~~~~
IF ERRORLEVEL 1 GOTO error
ECHO ~~~  Lua tests passed  ~~~
GOTO end
:error
ECHO ~~~  Lua tests failed  ~~~
:end
ECHO ~~~~~~~~~~~~~~~~~~~~~~~~~~
