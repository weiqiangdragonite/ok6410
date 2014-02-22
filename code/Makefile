#
# Makefile:
#
# By: weiqiangdragonite@gmail.com
#

# name for executable
EXE = ok6410

# space-separated list of source files
SRC_S = bsp/start.s \
	arm/arm_s.s \

SRC_C = bsp/exceptions.c \
	bsp/clock.c \
	bsp/sdram.c \
	bsp/nand.c \
	bsp/uart.c \
	bsp/boot.c \
	bsp/timer.c \
	bsp/led.c \
	kernel/core.c \
	kernel/task.c \
	kernel/time.c \
	lib/lib.c \
	app/app.c

# space-separated list of header files
HDRS = 

# automatically generated list of object files
OBJS = $(SRC_S:.s=.o) $(SRC_C:.c=.o)

# obj directory
OBJS_DIR = $(shell pwd)/os_obj

# compiler to use
PREFIX = arm-none-eabi
CC = $(PREFIX)-gcc
LD = $(PREFIX)-ld
CP = $(PREFIX)-objcopy
DP = $(PREFIX)-objdump

# include path: -I
INCLUDE = -I$(shell pwd)/bsp -I$(shell pwd)/kernel -I$(shell pwd)/lib

# lib path: -L
LIB_PATH = -L/usr/local/CodeSourcery/$(PREFIX)/lib/gcc/$(PREFIX)/4.8.1/

# lib: -l
LIB = -lgcc

# flags to pass compiler
# used for release: -fomit-frame-pointer -O2
# ARM GCC floating-point: -mfloat-abi=softfp -mfpu=vfp
CFLAGS = -Wall -mcpu=arm1176jzf-s

# default target
$(EXE).bin : $(OBJS)
	rm -rf $(OBJS_DIR); mkdir $(OBJS_DIR)
	cp $(OBJS) $(EXE).lds $(OBJS_DIR)
	cd $(OBJS_DIR); $(LD) -T $(EXE).lds *.o $(LIB_PATH) $(LIB) -o $(EXE).elf
	cd $(OBJS_DIR); $(CP) -O binary -S $(EXE).elf $@
	cd $(OBJS_DIR); $(DP) -D $(EXE).elf > $(EXE).dis
	cd $(OBJS_DIR); cp $(EXE).bin $(EXE).dis ../
	echo 'Compile done!'

# dependencies
%.o : %.s
	$(CC) -c $(CFLAGS) $^ -o $@

%.o : %.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@

# housekeeping
.PHONY : clean
clean:
	rm -rf $(OBJS_DIR)
	rm -f $(OBJS)
	rm -f *.dis *.bin
