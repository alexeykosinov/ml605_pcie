@echo off
call %XILINX%/../settings64.bat
mb-objcopy -O srec main/Debug/main.elf main/Debug/main.srec
xmd -nx -hw ml605/system.xml -tcl flashwriter.tcl ../etc/flash.tcl
pause
rm *.tmp
rm cmd.bat
rd /S /Q etc