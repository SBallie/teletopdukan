OUTPUT_FORMAT("binary")
ENTRY(_entry)
SECTIONS
{
  . = 0x100000;
  .text : {
    _text_start = .;
    *(.multiboot)
    