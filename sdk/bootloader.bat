@echo off
data2mem -bm ml605/system_bd.bmm -bt ml605/system.bit -bd bootloader/Debug/bootloader.elf tag MB_Core -o b ml605/bootloader.bit
promgen -w -p mcs -o ml605/bootloader -x xcf128x -data_width 16 -u 0 ml605/bootloader.bit
impact -batch ../system.impact
rm *.log
pause