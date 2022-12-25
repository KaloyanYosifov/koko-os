deploy: clean build see

build:
	test -d ./bin || mkdir bin
	nasm -f bin boot.asm -o ./bin/boot.bin

see:
	qemu-system-x86_64 -hda ./bin/boot.bin

check_bin:
	ndisasm ./bin/boot.bin

clean:
	rm -rf bin
