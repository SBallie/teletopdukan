OUTPUT_FORMAT("binary")
ENTRY(_entry)
SECTIONS
{
  . = 0x100000;
  .text : {
    _text_start = .;
    *(.multiboot)
    *(.text)
    _text_end = .;
    . = ALIGN(4096);
  }
  .rodata :
  {
    *(.rodata)
    . = ALIGN(4096);
  }
  .data :
  {
	_data_star