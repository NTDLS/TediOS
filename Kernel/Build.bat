@ECHO OFF

CD Source

SET PATH=C:\OCAL\DJGPP\Bin;%PATH%
SET DJGPP=C:\OCAL\DJGPP\DJGPP.env

Echo Compiling...
gxx -c General\*.cpp -I C:\DJGPP\include -ffreestanding -nostdlib -fno-builtin -fno-rtti -fno-exceptions
gxx -c Lib\*.cpp -I C:\DJGPP\include -ffreestanding -nostdlib -fno-builtin -fno-rtti -fno-exceptions
gxx -c Lib\*.s -I C:\DJGPP\include -ffreestanding -nostdlib -fno-builtin -fno-rtti -fno-exceptions
gxx -c Drivers\*.cpp -I C:\DJGPP\include -ffreestanding -nostdlib -fno-builtin -fno-rtti -fno-exceptions
gxx -c Apps\*.cpp -I C:\DJGPP\include -ffreestanding -nostdlib -fno-builtin -fno-rtti -fno-exceptions

REM Assembling with NASM 2.05.01 Win32
Echo Assembling...
C:\OCAL\NASM\32\NASM.exe -f aout -o start.obj General\Start.asm
C:\OCAL\NASM\32\NASM.exe -f aout -o setjmp.obj Lib\*.asm

Echo Linking...
REM The start.obj MUST be linked FIRST of the kernel will not work!!!!
ld -T link.ld -o Bin\KERNEL.BIN start.obj setjmp.obj *.o

Echo Cleaning...
Del *.o
Del *.obj

Pause
