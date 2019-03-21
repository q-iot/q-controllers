@set B="Q-Controllers"

for /f "tokens=2,*" %%i in ('reg query "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" /v "Desktop"') do @set "Desk=%%j"

for /f "tokens=1,2,3,4 delims=/- " %%a in ("%date%") do @set D=%%a%%b%%c_00

copy .\Output\%B%.bin %Desk\STM32F103CB_BIN\%B%_%D%.bin

