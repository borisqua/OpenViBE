@echo off
setlocal EnableDelayedExpansion
setlocal enableextensions 

SET InitEnvScript="win32-init_env_command.cmd"

call "!InitEnvScript!"

REM needed for debugging in VS ...
SET "OV_PATH_ROOT=%CD%\..\dist"
SET "OV_PATH_BIN=%OV_PATH_ROOT%\bin"
SET "OV_PATH_DATA=%OV_PATH_ROOT%\share\openvibe"
SET "OV_PATH_LIB=%OV_PATH_ROOT%\bin"
SET "PATH=%OV_PATH_ROOT%\bin;%PATH%"

REM Choose Visual Studio

REM Try the non-free version first ...
IF EXIST "%VSINSTALLDIR%\Common7\IDE\devenv.exe" (
    "%VSINSTALLDIR%\Common7\IDE\devenv.exe" ..\local-tmp\visual\OpenViBE.sln
	goto terminate
)

REM VS2012 Express
IF EXIST "%VSINSTALLDIR%\Common7\IDE\WDExpress.exe" (
	"%VSINSTALLDIR%\Common7\IDE\WDExpress.exe" ..\local-tmp\visual\OpenViBE.sln
	goto terminate
)

REM Default, e.g. 2010 Express
"%VSINSTALLDIR%\Common7\IDE\VCExpress.exe" ..\local-tmp\visual\OpenViBE.sln

:terminate