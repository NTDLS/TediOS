@Echo Off
COPY ..\Images\Base.vfd ..\Images\Boot.vfd /Y
C:\OCAL\Tools\fat_imgen.exe modify ..\Images\Boot.vfd -f Source\Bin\KERNEL.BIN
COPY ..\Images\Boot.vfd ..\Emulators\BOCHS\ /Y
COPY ..\Images\Boot.vfd ..\Emulators\VirtualPC\ /Y

:BOCHS
CD ..\Emulators\BOCHS
"C:\Program Files\Bochs\bochs.exe" -q -f Bochs.bxrc
GOTO End

:End
