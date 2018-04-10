@ECHO OFF
Cls

Echo Compiling...
tcc -LC:\OCAL\TC\LIB -IC:\OCAL\TC\INCLUDE Source\*.cpp

Echo Cleaning...
Del *.obj

Echo Creating test image...
COPY ..\Images\Empty.vfd Install.vfd /Y
C:\OCAL\Tools\fat_imgen.exe modify Install.vfd -f Sys.exe
