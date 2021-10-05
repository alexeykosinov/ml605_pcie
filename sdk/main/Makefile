# Custom makefile
MBGCC=D:\_files\Xilinx\14.7\ISE_DS\EDK\gnu\microblaze\nt\bin
export PATH:=$(MBGCC);$(PATH)

PRJ_NAME 		:= main
ACTIVE_CONFIG	:= Debug
C_SRCS			:= $(wildcard *.c)
O_SRCS			:= $(addprefix $(ACTIVE_CONFIG)/, $(patsubst %.c, %.o, $(C_SRCS)))
LD_SRCS 		:= lscript.ld
LIBS 			:= -Wl,--start-group,-lxil,-lgcc,-lc,--end-group
MB_OPT 			:= -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.50.c -mno-xl-soft-mul -Wl,--no-relax
BSP_DIR 		:= ../ml605_bsp/microblaze_0
RM 				:= rm -r
DIR_GUARD 		:= @mkdir -p $(ACTIVE_CONFIG)

all: $(O_SRCS) link check

$(ACTIVE_CONFIG)/%.o: %.c
	$(DIR_GUARD)
	mb-gcc -Wall -O0 -g3 -c -fmessage-length=0 -I $(BSP_DIR)/include $(MB_OPT) -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT "$(@:%.o=%.d)" -o "$@" "$^"

link: 
	mb-gcc -Wl,-T -Wl,$(LD_SRCS) -L $(BSP_DIR)/lib $(MB_OPT) -Wl,--gc-sections -o $(ACTIVE_CONFIG)/$(PRJ_NAME).elf $(O_SRCS) $(LIBS)

check: link
	mb-size $(ACTIVE_CONFIG)/$(PRJ_NAME).elf |tee "$(ACTIVE_CONFIG)/$(PRJ_NAME).size"
	elfcheck $(ACTIVE_CONFIG)/$(PRJ_NAME).elf -hw ../ml605/system.xml -pe microblaze_0 |tee "$(ACTIVE_CONFIG)/$(PRJ_NAME).elfcheck"

clean:
	$(RM) $(ACTIVE_CONFIG)

.PHONY: all clean
