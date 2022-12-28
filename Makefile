SOURCEDIR := src
BUILDDIR := build
BINDIR := bin
SOURCES := $(shell find $(SOURCEDIR) -name '*.c')
# get asm sources, except for special cases like kernel.asm and boot.asm
ASM_SOURCES := $(shell find $(SOURCEDIR) -name '*.asm' -not -path 'src/kernel.asm' -not -path 'src/boot/boot.asm')

# Kernel asm must be first as that is our entrypoint
OBJECTS := $(BUILDDIR)/kernel.asm.o $(addprefix $(BUILDDIR),$(ASM_SOURCES:$(SOURCEDIR)%.asm=%.asm.o)) $(addprefix $(BUILDDIR),$(SOURCES:$(SOURCEDIR)%.c=%.o))

# To print information
#$(info $$OBJECTS is [${ASM_SOURCES}])

BINARY := $(BUILDDIR)/os.bin
INCLUDES := -I ./src
FLAGS := -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

build: clean ./bin/boot.bin ./bin/kernel.bin
	dd if=./bin/boot.bin >> $(BINARY)
	dd if=./bin/kernel.bin >> $(BINARY)
	# put enought sectors to acommodate for a big kernel if it happens in the future
	dd if=/dev/zero bs=512 count=100 >> $(BINARY)

run:
	qemu-system-x86_64 -hda $(BINARY)

clean:
	rm -rf bin
	rm -rf build

debugger:
	x86_64-elf-gdb -ex "add-symbol-file ./build/kernelfull.o 0x00100000" -ex "target remote | qemu-system-x86_64 -hda $(BINARY) -S -gdb stdio"

$(BINDIR)/kernel.bin: $(OBJECTS)
	mkdir -p $(@D)
	i686-elf-ld -g -relocatable $(OBJECTS) -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o

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
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@
