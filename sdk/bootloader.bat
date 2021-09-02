@echo off
elfcheck -hw ml605/system.xml -mode bootload -mem BRAM -pe microblaze_0 bootloader/Debug/bootloader.elf 
data2mem -bm ml605/system_bd.bmm -bt ml605/system.bit -bd bootloader/Debug/bootloader.elf tag microblaze_0 -o b ml605/bootloader.bit
promgen -w -p mcs -o ml605/bootloader -x xcf128x -data_width 16 -u 0 ml605/bootloader.bit
impact -batch ../system.impact
rm *.log
pause