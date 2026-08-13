set -e

rm -f *.o *.bin *.elf *.img *.lock

nasm -f elf32 entry.asm -o entry.o
nasm -f elf32 interrupts.asm -o interrupts.o
nasm -f elf32 gdt.asm -o gdt.o

clang++ -target i386-pc-none-elf -ffreestanding -fno-exceptions -fno-rtti -fno-pie -c kernel.cpp -o kernel.o
clang++ -target i386-pc-none-elf -ffreestanding -fno-exceptions -fno-rtti -fno-pie -c teclado.cpp -o teclado.o
clang++ -target i386-pc-none-elf -ffreestanding -fno-exceptions -fno-rtti -fno-pie -c idt.cpp -o idt.o

ld.lld -m elf_i386 -nostdlib -T linker.ld entry.o interrupts.o gdt.o kernel.o teclado.o idt.o -o kernel.elf
objcopy -O binary kernel.elf kernel.bin

nasm -f bin boot.asm -o boot.bin

dd if=/dev/zero of=poteos.img bs=1M count=1 status=none
dd if=boot.bin of=poteos.img bs=512 count=1 conv=notrunc status=none
dd if=kernel.bin of=poteos.img bs=512 seek=1 conv=notrunc status=none

pkill -9 termux-x11 2>/dev/null || true
termux-x11 :0 >/dev/null 2>&1 &
sleep 4
export DISPLAY=:0

bochs -q -f bochsrc.txt
