# Flash programmer parameters file
set FLASH_FILE              "main/Debug/main.srec"
set FLASH_BASEADDR          0x42000000; 
set FLASH_PROG_OFFSET       0x00F00000; # Offset for elf
set FLASH_BUSWIDTH          16;
set SCRATCH_BASEADDR        0xD0000000; # DDR Base address
set SCRATCH_LEN             0x10000000;
set TARGET_TYPE             "MICROBLAZE";
set PROC_INSTANCE           "MB_Core";
set EXTRA_COMPILER_FLAGS    "-mxl-barrel-shift -mno-xl-soft-mul -mxl-pattern-compare -mlittle-endian";
set XMD_CONNECT             "connect mb mdm -debugdevice cpunr 1";
set XILINX_PLATFORM_FLASH   1
set ENDIANNESS              "LE"
