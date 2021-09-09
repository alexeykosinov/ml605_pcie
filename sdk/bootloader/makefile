# Custom makefile
MBGCC=D:\_files\Xilinx\14.7\ISE_DS\EDK\gnu\microblaze\nt\bin
export PATH:=$(MBGCC);$(PATH)


PRJ_NAME := bootloader
LD_SRCS := lscript.ld
LIBS := -Wl,--start-group,-lxil,-lgcc,-lc,--end-group
MB_OPT := -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.50.c -mno-xl-soft-mul -Wl,--no-relax
BSP_DIR := ../ml605_bsp/microblaze_0
RM := rm -rf

all: build link check

build: 
	mb-gcc -Wall -Os -c -fmessage-length=0 -I $(BSP_DIR)/include $(MB_OPT) -ffunction-sections -fdata-sections -MMD -MP -MF $(PRJ_NAME).d -MT $(PRJ_NAME).d -o $(PRJ_NAME).o $(PRJ_NAME).c

link: 
	mb-gcc -Wl,-T -Wl,$(LD_SRCS) -L $(BSP_DIR)/lib $(MB_OPT) -Wl,--gc-sections -o $(PRJ_NAME).elf $(PRJ_NAME).o $(LIBS)

check: link
	mb-size $(PRJ_NAME).elf |tee "$(PRJ_NAME).size"
	elfcheck $(PRJ_NAME).elf -hw ../ml605/system.xml -pe microblaze_0 |tee "$(PRJ_NAME).elfcheck"

clean:
	$(RM) *.d
	$(RM) *.o
	$(RM) *.elf
	$(RM) *.elfcheck
	$(RM) *.size

.PHONY: all clean
