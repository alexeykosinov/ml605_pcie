@echo off
elfcheck -hw ml605/system.xml -mode bootload -mem BRAM -pe microblaze_0 bootloader/Debug/bootloader.elf 
data2mem -bm ml605/system_bd.bmm -bt ml605/system.bit -bd bootloader/Debug/bootloader.elf tag microblaze_0 -o b ml605/bootloader.bit
promgen -w -p mcs -c FF -o ml605/bootloader -x xcf128x -u 0 ml605/bootloader.bit -bpi_dc parallel -data_width 16  
impact -batch ../system.impact
rm *.log
pause