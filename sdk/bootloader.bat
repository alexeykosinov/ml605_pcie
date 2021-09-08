@echo off
elfcheck -hw ml605/system.xml -mode bootload -mem BRAM -pe microblaze_0 bootloader/bootloader.elf 
data2mem -bm ml605/system_bd.bmm -bt ml605/system.bit -bd bootloader/bootloader.elf tag microblaze_0 -o b ml605/bootloader.bit
impact -batch ../system.impact
rm *.log
rm ml605/*_cclktemp.bit
pause