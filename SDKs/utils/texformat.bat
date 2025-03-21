@echo off
set file=%1

for /f "tokens=2 delims== " %%A in (
  '"%~dp0texdiag.exe info %file% | findstr format"'
) do echo %%A