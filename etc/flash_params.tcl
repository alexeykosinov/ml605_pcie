set FLASH_FILE             "../system.mhs";
set FLASH_BASEADDR         0x42000000;	# Base address of flash device
set FLASH_PROG_OFFSET      0x00000000;	# Offset at which the image should be programmed within flash
set FLASH_BUSWIDTH         16;			# Device bus width of all flash parts combined
set SCRATCH_BASEADDR       0x00000000;	# Base address of scratch memory
set SCRATCH_LEN            0x00010000;	# Length of scratch memory
set EXTRA_COMPILER_FLAGS   "-mxl-barrel-shift -mno-xl-soft-mul -mxl-pattern-compare";
set XMD_CONNECT            "connect mb mdm -debugdevice cpunr 1";
set TARGET_TYPE            "MICROBLAZE";
set PROC_INSTANCE          "MB_Core";
set XILINX_PLATFORM_FLASH  1
