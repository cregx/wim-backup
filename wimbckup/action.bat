@echo off
REM
REM This script is responsible for performing a data backup and a data restore.
REM Version 1.0
REM December 2022
REM
REM Copyright 2022 Christoph Regner
REM
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM http://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.
REM
REM Parameter description:
REM  Used parameter: %1
REM   Represents one of the action types:  Backup | Recovery
REM  Used parameter: %2
REM   Returns the full path to the backup | data recovery file (*.wim).
REM  Used parameter: %3 (for backup only)
REM   Represents the volume that is to be backed up, e.g. C:\
REM 
REM Return values:
REM  The script must return the value 0x400 (1024) in case of successful processing.
REM  In case of an failure the script should return a different value (!= 0x400).
REM  This will then be displayed as an error in the GUI (wimbckup.exe)
REM  after processing is complete.
REM
REM  Note: The return value of 0x0 (0) represents an error.
REM  The return value 0xFFFFFFFFFFFFFFFF (-1) as well as 0xC000013A (3221225786)
REM  are reserved for internal purposes and therefore must not be used.

SET /A E_SUCCESS=0x400
SET /A errno^|=%E_SUCCESS%
 
echo Operation: %1 %2 %3

echo Systemdrive:
cd %systemdrive%

REM Path to the Logfile.
SET LOGFILE=%systemdrive%\Tools\dism.log
echo Logfile: %LOGFILE%

REM Start backup or data recovery.
if "%1" == "Backup" (
 dism /Capture-Image /ImageFile:"%2" /CaptureDir:%3 /Name:"WinOS" /ea /LogPath:%LOGFILE% /LogLevel:1
 
 REM Check if an error occurred during the execution of dism.
 REM In this case, the log file contains entries with the term "Error". 
 findstr /i "^, Error" %LOGFILE%
 if %errorlevel% neq 0 (
  EXIT /B %errorlevel%
 )
) else (
 diskpart /s %systemdrive%\Tools\diskpart.txt
 dism /Apply-Image /ImageFile:"%2" /Index:1 /ApplyDir:W:\ /LogPath:%LOGFILE% /LogLevel:1
 W:\Windows\System32\Bcdboot W:\Windows

 REM Check if an error occurred during the execution of dism.
 REM In this case, the log file contains entries with the term "Error".
 findstr /i "^, Error" %LOGFILE%
 if %errorlevel% neq 0 (
  EXIT /B %errorlevel%
 )

 REM After the restore process, the ReAgent.xml must be deleted for systems that use BitLocker.
 if "%1" == "Recovery" (
  SET REAGENTFILE="W:\Windows\System32\Recovery\ReAgent.xml"
  if EXIST "%REAGENTFILE%" (
     del "%REAGENTFILE%"
  )
 )
)
EXIT /B %errno%