PREFIX  = riscv64-unknown-elf-
CFLAGS  = -march=rv32i -mabi=ilp32 -O1 -nostdlib -nostartfiles -ffreestanding -Wall
all: prog.hex prog.dump

prog.elf: start.S main.c link.ld
	$(PREFIX)gcc $(CFLAGS) -T link.ld start.S main.c -o $@ -Wl,--no-warn-rwx-segments
prog.bin: prog.elf
	$(PREFIX)objcopy -O binary $< $@
prog.hex: prog.bin
	python3 bin2hex.py $< $@
prog.dump: prog.elf
	$(PREFIX)objdump -d -M no-aliases $< > $@
clean:
	rm -f prog.elf prog.bin prog.hex prog.dump
