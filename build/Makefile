TOOLCHAIN_PREFIX=aarch64-none-elf
CC=$(TOOLCHAIN_PREFIX)-gcc

SRCS = $(wildcard *.cc)
OBJS = $(SRCS:.cc=.o)
CFLAGS = -Wall -Wextra -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

KERNEL_NAME=kernel8

# Locations of the files
KER_SRC = ../src/kernel
KER_HEAD = ../include
COMMON_SRC = ../src/common
OBJ_DIR = objects
KERSOURCES = $(wildcard $(KER_SRC)/*.cc)
COMMONSOURCES = $(wildcard $(COMMON_SRC)/*.cc)
ASMSOURCES = $(wildcard $(KER_SRC)/*.S)
OBJECTS += $(patsubst $(KER_SRC)/%.cc, $(OBJ_DIR)/%.o, $(KERSOURCES))
OBJECTS += $(patsubst $(COMMON_SRC)/%.cc, $(OBJ_DIR)/%.o, $(COMMONSOURCES))
OBJECTS += $(patsubst $(KER_SRC)/%.S, $(OBJ_DIR)/%.o, $(ASMSOURCES))
OBJECTS += font_psf.o font_sfn.o

HEADERS = $(wildcard $(KER_HEAD)/*.h) $(wildcard $(KER_HEAD)/*.hh)

build: $(OBJECTS) $(HEADERS)
	$(TOOLCHAIN_PREFIX)-ld -nostdlib -nostartfiles $(OBJECTS) -T linker.ld -o $(KERNEL_NAME).elf
	$(TOOLCHAIN_PREFIX)-objcopy -O binary $(KERNEL_NAME).elf $(KERNEL_NAME).img

$(OBJ_DIR)/%.o: $(KER_SRC)/%.cc
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@ $(CSRCFLAGS)

$(OBJ_DIR)/%.o: $(KER_SRC)/%.S
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KER_SRC) -c $< -o $@

$(OBJ_DIR)/%.o: $(COMMON_SRC)/%.cc
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@ $(CSRCFLAGS)

font_psf.o: font.psf
	$(TOOLCHAIN_PREFIX)-ld -r -b binary -o font_psf.o font.psf

font_sfn.o: font.sfn
	$(TOOLCHAIN_PREFIX)-ld -r -b binary -o font_sfn.o font.sfn

clean:
	rm -rf $(OBJ_DIR) *.o
	rm $(KERNEL_NAME).elf
	rm $(KERNEL_NAME).img

run: build
	qemu-system-aarch64 -M raspi3 -kernel $(KERNEL_NAME).img -serial stdio
