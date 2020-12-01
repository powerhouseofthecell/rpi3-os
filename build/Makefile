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
KER_HEAD = ../include

KER_SRC = ../src/kernel
COMMON_SRC = ../src/common
USR_SRC = ../src/usr

OBJ_DIR = objects

# source files
KERSOURCES = $(wildcard $(KER_SRC)/*.cc)
COMMONSOURCES = $(wildcard $(COMMON_SRC)/*.cc)
ASMSOURCES = $(wildcard $(KER_SRC)/*.S)
USRSOURCES = $(wildcard $(USR_SRC)/*.cc)

# object files
OBJECTS += $(patsubst $(KER_SRC)/%.cc, $(OBJ_DIR)/%.o, $(KERSOURCES))
OBJECTS += $(patsubst $(COMMON_SRC)/%.cc, $(OBJ_DIR)/%.o, $(COMMONSOURCES))
OBJECTS += $(patsubst $(KER_SRC)/%.S, $(OBJ_DIR)/%.o, $(ASMSOURCES))
OBJECTS += $(patsubst $(USR_SRC)/%.cc, $(OBJ_DIR)/%.o, $(USRSOURCES))
OBJECTS += font_psf.o

HEADERS = $(wildcard $(KER_HEAD)/*.h) $(wildcard $(KER_HEAD)/*.hh)

build: $(OBJECTS) $(HEADERS)
	$(TOOLCHAIN_PREFIX)-ld $(LINKERFLAGS) $(OBJECTS) -T linker.ld -o $(KERNEL_NAME).elf
	$(TOOLCHAIN_PREFIX)-objcopy -O binary $(KERNEL_NAME).elf $(KERNEL_NAME).img

# rule for making kernel source objects
$(OBJ_DIR)/%.o: $(KER_SRC)/%.cc
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@ $(CSRCFLAGS)

# rule for kernel source assembly objects
$(OBJ_DIR)/%.o: $(KER_SRC)/%.S
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@

# rule for common source objects
$(OBJ_DIR)/%.o: $(COMMON_SRC)/%.cc
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(KER_SRC) -I$(KER_HEAD) -c $< -o $@ $(CSRCFLAGS)

# rule for userland source objects
$(OBJ_DIR)/%.o: $(USR_SRC)/%.cc
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(USR_SRC) -I$(KER_HEAD) -c $< -o $@ $(CSRCFLAGS)

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
