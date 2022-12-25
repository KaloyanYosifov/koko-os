deploy: clean build see

build:
	test -d ./bin || mkdir bin
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

see:
	qemu-system-x86_64 -hda ./bin/boot.bin

check_bin:
	ndisasm ./bin/boot.bin

clean:
	rm -rf bin

debugger:
	x86_64-elf-gdb -ex "target remote | qemu-system-x86_64 -hda ./bin/boot.bin -S -gdb stdio"
