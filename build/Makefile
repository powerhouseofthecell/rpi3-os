TOOLCHAIN_PREFIX=aarch64-none-elf
CC=$(TOOLCHAIN_PREFIX)-gcc

SRCS = $(wildcard *.cc)
OBJS = $(SRCS:.cc=.o)
CFLAGS = -O2 -std=gnu++1z -ffreestanding -nostdinc -nostdlib -nostartfiles -g -ggdb \
	-Wall -W -Wshadow -Wno-format -Wno-unused-parameter -Wextra
	# -fno-omit-frame-pointer -fno-pic \
	# -fno-stack-protector \
	# -Wstack-usage=1024

LINKERFLAGS = -Os --gc-sections -z max-page-size=0x1000 -static -nostdlib -nostartfiles

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
OBJECTS += font_psf.o

HEADERS = $(wildcard $(KER_HEAD)/*.h) $(wildcard $(KER_HEAD)/*.hh)

build: $(OBJECTS) $(HEADERS)
	$(TOOLCHAIN_PREFIX)-ld $(LINKERFLAGS) $(OBJECTS) -T linker.ld -o $(KERNEL_NAME).elf
	$(TOOLCHAIN_PREFIX)-objcopy -j .text -j .rodata -j .data -j .bss -j .ctors -j .init_array -O binary $(KERNEL_NAME).elf $(KERNEL_NAME).img

$(OBJ_DIR)/%.o: $(KER_SRC)/%.cc
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@ $(CSRCFLAGS)

$(OBJ_DIR)/%.o: $(KER_SRC)/%.S
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@

$(OBJ_DIR)/%.o: $(COMMON_SRC)/%.cc
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@ $(CSRCFLAGS)

font_psf.o: font.psf
	$(TOOLCHAIN_PREFIX)-ld -r -b binary -o font_psf.o font.psf

clean:
	rm -rf $(OBJ_DIR) *.o
	rm $(KERNEL_NAME).elf
	rm $(KERNEL_NAME).img

run: build
	qemu-system-aarch64 -M raspi3 -kernel $(KERNEL_NAME).img -serial stdio

run-debug: build
	qemu-system-aarch64 -M raspi3 -kernel $(KERNEL_NAME).img -serial stdio -d int

run-gdb: build
	qemu-system-aarch64 -M raspi3 -kernel $(KERNEL_NAME).img -serial stdio -s -S -d int
