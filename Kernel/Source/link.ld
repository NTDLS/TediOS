OUTPUT_FORMAT("binary")
ENTRY(start)
phys = 0x00100000;
SECTIONS
{
  .text phys : AT(phys) {
    __code = .;
    *(.text)
    *(.rodata)
    . = ALIGN(4096);
  }
  .data : AT(phys + (__data - __code))
  {
    __data = .;
    *(.data)
    . = ALIGN(4096);
  }
  .bss : AT(phys + (__bss - __code))
  {
    __bss = .;
    *(.bss)
    . = ALIGN(4096);
  }
  __end = .;
}