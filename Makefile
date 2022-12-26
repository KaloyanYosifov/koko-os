SOURCEDIR := src
BUILDDIR := build
BINDIR := bin
SOURCES := $(shell find $(SOURCEDIR) -name '*.c')
OBJECTS := $(BUILDDIR)/kernel.asm.o $(addprefix $(BUILDDIR),$(SOURCES:$(SOURCEDIR)%.c=%.o))
BINARY := $(BUILDDIR)/os.bin
INCLUDES := -I ./src
FLAGS := -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

FILES := ./build/kernel.asm.o ./build/kernel.o ./build/lib/terminal.o ./build/lib/string.o

build: clean build-reqs ./bin/boot.bin ./bin/kernel.bin
	dd if=./bin/boot.bin >> $(BINARY)
	dd if=./bin/kernel.bin >> $(BINARY)
	# put enought sectors to acommodate for a big kernel if it happens in the future
	dd if=/dev/zero bs=512 count=100 >> $(BINARY)

run:
	qemu-system-x86_64 -hda $(BINARY)

check_bin:
	ndisasm ./bin/boot.bin

clean:
	rm -rf bin
	rm -rf build

debugger:
	x86_64-elf-gdb -ex "add-symbol-file ./build/kernelfull.o 0x00100000" -ex "target remote | qemu-system-x86_64 -hda $(BINARY) -S -gdb stdio"

build-reqs:
	test -d ./bin || mkdir ./bin
	test -d ./build || mkdir -p ./build/lib

$(BINDIR)/kernel.bin: $(OBJECTS)
	i686-elf-ld -g -relocatable $(OBJECTS) -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o

$(BINDIR)/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

$(BUILDDIR)/kernel.asm.o: ./src/kernel.asm
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@
