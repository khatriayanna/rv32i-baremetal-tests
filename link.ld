/* Single memory starting at 0x0 (code + data + stack). Change LENGTH to match your memory. */
MEMORY { RAM (rwx) : ORIGIN = 0x00000000, LENGTH = 16K }
SECTIONS
{
  .text : { *(.text.init) *(.text*) } > RAM
  .rodata : { *(.rodata*) } > RAM
  .data : { *(.data*) } > RAM
  .bss : { *(.bss*) *(COMMON) } > RAM
  _stack_top = ORIGIN(RAM) + LENGTH(RAM);
}
