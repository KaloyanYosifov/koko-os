SOURCEDIR := src
BUILDDIR := build
BINDIR := bin
SOURCES := $(shell find $(SOURCEDIR) -name '*.c')
VIRTUAL_MACHINE := qemu-system-i386
# get asm sources, except for special cases like kernel.asm and boot.asm
ASM_SOURCES := $(shell find $(SOURCEDIR) -name '*.asm' -not -path 'src/kernel.asm' -not -path 'src/boot/boot.asm')
SYMBOLS_FILE := $(BUILDDIR)/kernelfull-elf.o
DEFAULT_LINK_KERNEL_FILE := $(BUILDDIR)/kernel-default-link.o

# Kernel asm must be first as that is our entrypoint
OBJECTS := $(BUILDDIR)/kernel.asm.o $(addprefix $(BUILDDIR),$(ASM_SOURCES:$(SOURCEDIR)%.asm=%.asm.o)) $(addprefix $(BUILDDIR),$(SOURCES:$(SOURCEDIR)%.c=%.o))

# To print information
#$(info $$OBJECTS is [${ASM_SOURCES}])

BINARY := $(BINDIR)/os.bin
FLAGS := -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

build: clean ./bin/boot.bin ./bin/kernel.bin
	dd if=./bin/boot.bin >> $(BINARY)
	dd if=./bin/kernel.bin >> $(BINARY)
	# put enought sectors to acommodate for a big kernel if it happens in the future
	dd if=/dev/zero bs=1048576 count=16 >> $(BINARY)

run:
	$(VIRTUAL_MACHINE) -hda $(BINARY)

add_file_to_os_bin:
	docker run --name koko_os_docker --rm -d --privileged -v $$(pwd)/$(BINDIR):/app fedora:32 bash -c "cd /app && mkdir -p /app/test-data && mkdir -p /tmp/testing && sudo mount -t vfat ./os.bin /tmp/testing && sleep 99999"
	docker cp "$$(pwd)/test-data" koko_os_docker:/app/
	docker exec koko_os_docker bash -c "cp -R /app/test-data/* /tmp/testing/"
	docker stop koko_os_docker > /dev/null &

clean:
	rm -rf bin
	rm -rf build

debugger:
	x86_64-elf-gdb -ex "add-symbol-file $(SYMBOLS_FILE) 0x00100000" -ex "target remote | $(VIRTUAL_MACHINE) -hda $(BINARY) -S -gdb stdio"

$(BINDIR)/kernel.bin: $(OBJECTS)
	mkdir -p $(@D)
	i686-elf-ld -g -relocatable $(OBJECTS) -o $(DEFAULT_LINK_KERNEL_FILE)
	i686-elf-gcc $(FLAGS) -T ./src/linker-symbol.ld -o $(SYMBOLS_FILE) $(DEFAULT_LINK_KERNEL_FILE)
	i686-elf-gcc $(FLAGS) -T ./src/linker-binary.ld -o ./bin/kernel.bin $(DEFAULT_LINK_KERNEL_FILE)

$(BINDIR)/boot.bin: ./src/boot/boot.asm
	mkdir -p $(@D)
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

$(BUILDDIR)/kernel.asm.o: ./src/kernel.asm
	mkdir -p $(@D)
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

$(BUILDDIR)/%.asm.o: $(SOURCEDIR)/%.asm
	mkdir -p $(@D)
	nasm -f elf -g $< -o $@

# See https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html for $< and $@
$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	mkdir -p $(@D)
	i686-elf-gcc $(FLAGS) -std=gnu99 -c $< -o $@
