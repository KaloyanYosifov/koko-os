deploy: clean build see

build:
	test -d ./bin || mkdir bin
	nasm -f bin boot.asm -o ./bin/boot.bin

see:
	qemu-system-x86_64 -hda ./bin/boot.bin

check_bin:
	ndisasm ./bin/boot.bin

burn_data:
	dd if=./data/message.txt >> ./bin/boot.bin
	dd if=/dev/zero bs=$$((512 - $$(stat -f%z ./data/message.txt))) count=1 >> ./bin/boot.bin
	dd if=./data/message2.txt >> ./bin/boot.bin
	dd if=/dev/zero bs=$$((512 - $$(stat -f%z ./data/message2.txt))) count=1 >> ./bin/boot.bin

clean:
	rm -rf bin
