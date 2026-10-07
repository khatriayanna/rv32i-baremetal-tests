# RV32I Bare-Metal C Test Suite

Small C programs that run **directly on a custom single-cycle RV32I RISC-V processor** (written in Verilog), with **no operating system**. The testbench loads the compiled program into memory, lets the core run it, and reports **PASS / FAIL** automatically.

## Why this project

Testing a CPU by hand-writing assembly is slow and hard to read. Real chip teams write tests in C, run them on the hardware design (in simulation, on FPGA, or on silicon), and check the results automatically. This project builds that flow end to end for my own RISC-V core.

## How it works (simple version)

```
 main.c + start.S + link.ld
          |  (RISC-V GCC)
          v
      prog.elf  -->  prog.bin  -->  prog.hex   (one 32-bit word per line)
                                       |
                                       |  $readmemh in the testbench
                                       v
                     +----------------------------------+
                     |  RV32I core (Verilog) + memory   |
                     +----------------------------------+
                         |                     |
          store to 0x10000004          store to 0x10000000
          (testbench prints a char)    (testbench prints PASS/FAIL and stops)
```

1. **`start.S`** runs first. It sets the stack pointer, calls `main()`, and parks the core when `main` returns. (Normally an OS or C runtime does this; here we do it ourselves.)
2. **`link.ld`** tells the linker where code, data, and the stack live in memory (starting at address `0x0`).
3. **`main.c`** runs four tests and prints one line for each.
4. **`Makefile`** compiles everything into `prog.elf`, turns it into a raw binary, then into `prog.hex`.
5. **`bin2hex.py`** converts the binary into the text format that Verilog's `$readmemh` can load.
6. The **testbench** (`tb_snippet.sv`) loads `prog.hex`, watches the core's data-memory writes, and reacts to two special addresses.

### Memory-mapped I/O

"Memory-mapped" means that writing to a special address makes something happen, instead of just storing data.

| Address       | Name    | What a store does                                          |
|---------------|---------|------------------------------------------------------------|
| `0x1000_0004` | UART TX | Testbench prints the low byte as a character               |
| `0x1000_0000` | Mailbox | `1` = all tests passed, `2` = something failed; sim ends   |

### The tests

| Test         | What it checks                                         |
|--------------|--------------------------------------------------------|
| `array_sum`  | loads, adds, loops, and branches                       |
| `fib10`      | register use and loop control                          |
| `mem_rw`     | store/load round trip through data memory              |
| `custom_mac` | the custom MAC instruction against a C reference value |

### The custom MAC instruction

RISC-V reserves some opcodes for custom use. The test calls my accelerator with the assembler directive `.insn`:

```c
__asm__ volatile (".insn r 0x0B, 0x0, 0x00, %0, %1, %2" : "=r"(rd) : "r"(a), "r"(b));
```

`0x0B` is the "custom-0" opcode. The result is compared with `mac_ref()` in C. If your accelerator uses another opcode or behaves differently, edit `CUSTOM_MAC` and `mac_ref()` in `main.c`.

## Requirements

- RISC-V bare-metal GCC (`riscv64-unknown-elf-gcc` works; the Makefile builds for `-march=rv32i -mabi=ilp32`)
- Python 3
- A Verilog/SystemVerilog simulator (I use Vivado)

## Build and run

```bash
make            # builds prog.elf, prog.bin, prog.hex, prog.dump
```

1. In your testbench, load the program: `initial $readmemh("prog.hex", dut.imem.mem);` (adjust the path to your memory array).
2. Add the UART/mailbox monitor from `tb_snippet.sv` (rename the signals to match your core).
3. Run the simulation.

`prog.dump` is the disassembly. It is useful to see exactly which instructions your core must execute.

## Expected output

```
RV32I bare-metal tests
array_sum: PASS
fib10: PASS
mem_rw: PASS
custom_mac: PASS
*** ALL TESTS PASSED ***
```

## Results

_Add your simulation log and waveform screenshot here after running on your core._

## Files

| File             | Purpose                                              |
|------------------|------------------------------------------------------|
| `start.S`        | startup code: stack pointer, call `main`, park       |
| `link.ld`        | memory layout (16 KB starting at 0x0)                |
| `main.c`         | the four tests, UART print, mailbox write            |
| `Makefile`       | build flow                                           |
| `bin2hex.py`     | binary to `$readmemh` format                         |
| `tb_snippet.sv`  | testbench code: load program, watch UART and mailbox |

## What I learned

- How a C program becomes machine code that a hardware core can run (compile, link, load).
- Why a linker script and startup code are needed when there is no OS.
- How memory-mapped I/O lets software talk to hardware and to a testbench.
- How to check a custom instruction against a software reference.

## Limitations and future work

- Single shared memory starting at address 0 (code, data, and stack together).
- No interrupts, no DMA, no caches yet. A timer interrupt and a small DMA block are the next steps.
- Companion project: [`rv32i-iss-reference-model`](https://github.com/khatriayanna/rv32i-iss-reference-model), a C++ simulator that runs the same program for comparison.
