/* Default linker script, for normal executables */

/* source, modified:
 *  /opt/CodeSourcery/Sourcery_G++_Lite/mips-sde-elf/lib/ldscripts/elf32ltsmip.x
 */
#ifdef __mips64
OUTPUT_FORMAT("elf64-tradlittlemips", "elf64-tradbigmips",
	      "elf64-tradlittlemips")
#else
OUTPUT_FORMAT("elf32-tradlittlemips", "elf32-tradbigmips",
	      "elf32-tradlittlemips")
#endif
OUTPUT_ARCH(mips)
ENTRY(__start)

PHDRS
{
  phdr PT_PHDR PHDRS;
  ro PT_LOAD FILEHDR PHDRS;
  rw PT_LOAD;
  tls PT_TLS;
  L4_DEFINE_L4PHDRS
}

SECTIONS
{
  PROVIDE (__executable_start = 0x010000);
  . = __executable_start + SIZEOF_HEADERS;
  /* No program code/data before _stext/_prog_img_start! */

  /* Read-only sections, merged into text segment: */
  /* PROVIDE (__executable_start = SEGMENT_START("text-segment", 0x0400000)); . = SEGMENT_START("text-segment", 0x0400000) + SIZEOF_HEADERS;
   */
  /DISCARD/ :
  { *(.interp)
    *(.MIPS.abiflags)
    *(.note.gnu.build-id)
    *(.hash)
    *(.gnu.hash)
    *(.dynsym)
    *(.dynstr)
    *(.gnu.version)
    *(.gnu.version_d)
    *(.gnu.version_r)
    *(.rel.init)
    *(.rela.init)
    *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*)
    *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*)
    *(.rel.fini)
    *(.rela.fini)
    *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*)
    *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*)
    *(.rel.data.rel.ro* .rel.gnu.linkonce.d.rel.ro.*)
    *(.rela.data.rel.ro* .rela.gnu.linkonce.d.rel.ro.*)
    *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*)
    *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*)
    *(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*)
    *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*)
    *(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*)
    *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*)
    *(.rel.ctors)
    *(.rela.ctors)
    *(.rel.dtors)
    *(.rela.dtors)
    *(.rel.got)
    *(.rela.got)
    *(.rel.dyn)
    *(.rel.sdata .rel.sdata.* .rel.gnu.linkonce.s.*)
    *(.rela.sdata .rela.sdata.* .rela.gnu.linkonce.s.*)
    *(.rel.sbss .rel.sbss.* .rel.gnu.linkonce.sb.*)
    *(.rela.sbss .rela.sbss.* .rela.gnu.linkonce.sb.*)
    *(.rel.sdata2 .rel.sdata2.* .rel.gnu.linkonce.s2.*)
    *(.rela.sdata2 .rela.sdata2.* .rela.gnu.linkonce.s2.*)
    *(.rel.sbss2 .rel.sbss2.* .rel.gnu.linkonce.sb2.*)
    *(.rela.sbss2 .rela.sbss2.* .rela.gnu.linkonce.sb2.*)
    *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*)
    *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
    *(.rel.ifunc)
    *(.rela.ifunc)
    *(.rel.plt)
    *(.rela.plt)
    *(.reginfo)
    *(.dynamic) }
  .text           :
  {
    _stext = .;
    _ftext = .;
    _prog_img_start = _ftext;
    *(.text.init) 
    *(.init)
    *(.text .text.* .gnu.linkonce.t.*)
    KEEP (*(.text.*personality*))
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.mips16.fn.*) *(.mips16.call.*)
  } :ro = 0
  .init           :
  {
    KEEP (*(.init))
  } :ro = 0
  .plt            : { *(.plt) *(.iplt)}
  .fini           :
  {
    KEEP (*(.fini))
  } = 0
  . = ALIGN(0x40);
  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);
  .rol4re_elf_aux : {
    KEEP (*(.rol4re_elf_aux))
  } : ro : l4re_aux
  .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) } :ro
  .rodata1        : { *(.rodata1) }
  .sdata2         :
  {
    *(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
  }
  .sbss2          : { *(.sbss2 .sbss2.* .gnu.linkonce.sb2.*) }
  .eh_frame_hdr : { *(.eh_frame_hdr) *(.eh_frame_entry .eh_frame_entry.*) } :ro
  .eh_frame       : ONLY_IF_RO { KEEP (*(.eh_frame)) *(.eh_frame.*) } :ro
  .gcc_except_table   : ONLY_IF_RO { *(.gcc_except_table .gcc_except_table.*) } :ro
  .gnu_extab   : ONLY_IF_RO { *(.gnu_extab*) } :ro
  . = ALIGN(CONSTANT(MAXPAGESIZE));
  /* Adjust the address for the data segment.  We want to adjust up to
     the same address within the page on the next page up.  */
  . = ALIGN (CONSTANT (MAXPAGESIZE)) - ((CONSTANT (MAXPAGESIZE) - .) & (CONSTANT (MAXPAGESIZE) - 1)); . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
  /* Exception handling  */
  PROVIDE (_sdata = .);
  .eh_frame       : ONLY_IF_RW { KEEP (*(.eh_frame)) *(.eh_frame.*) } :rw
  .gnu_extab      : ONLY_IF_RW { *(.gnu_extab) } :rw
  .gcc_except_table   : ONLY_IF_RW { *(.gcc_except_table .gcc_except_table.*) } :rw
  /* Thread Local Storage sections  */
  .tdata	  : { *(.tdata .tdata.* .gnu.linkonce.td.*) } :rw : tls
  .tbss		  : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } : rw
  .init_array     :
  {
     PROVIDE_HIDDEN (__init_array_start = .);
     KEEP (*(SORT(.init_array.*)))
     KEEP (*(.init_array))
     PROVIDE_HIDDEN (__init_array_end = .);
  }
  .fini_array     :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }
  /* ensure that data starts at a new L4 page */
  . = ALIGN(4K);
  .ctors          :
  {
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  } :rw
  . = ALIGN(4);
  .dtors          :
  {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  }
  .jcr            : { KEEP (*(.jcr)) }
  .data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro* .gnu.linkonce.d.rel.ro.*) }
  . = DATA_SEGMENT_RELRO_END (0, .);
  .data           :
  {
    _fdata = . ;
    *(.data .data.* .gnu.linkonce.d.*)
    KEEP (*(.gnu.linkonce.d.*personality*))
    SORT(CONSTRUCTORS)
  }
  .data1          : { *(.data1) }
  .got.plt        : { *(.got.plt) }
  . = .;
  _gp = ALIGN(16) + 0x7ff0;
  .got            : { *(.got) }
  /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
  .sdata          :
  {
    *(.sdata .sdata.* .gnu.linkonce.s.*)
  }
  .lit8           : { *(.lit8) }
  .lit4           : { *(.lit4) }
  _edata = .; PROVIDE (edata = .);
  __bss_start = .;
  _fbss = .;
  .sbss           :
  {
    *(.dynsbss)
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
  }
  .bss            :
  {
   *(.dynbss)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections.
      FIXME: Why do we need it? When there is no .bss section, we don't
      pad the .data section.  */
   . = ALIGN(. != 0 ? __SIZEOF_LONG__ : 1);
  }
  . = ALIGN(__SIZEOF_LONG__);
  . = ALIGN(__SIZEOF_LONG__);
  _end = .; PROVIDE (end = .);
  . = DATA_SEGMENT_END (.);
  . = ALIGN(CONSTANT(MAXPAGESIZE));
  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* DWARF 3 */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }
  .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
  .gptab.sdata : { *(.gptab.data) *(.gptab.sdata) }
  .gptab.sbss : { *(.gptab.bss) *(.gptab.sbss) }
  .mdebug.abi32 : { KEEP(*(.mdebug.abi32)) }
  .mdebug.abiN32 : { KEEP(*(.mdebug.abiN32)) }
  .mdebug.abi64 : { KEEP(*(.mdebug.abi64)) }
  .mdebug.abiO64 : { KEEP(*(.mdebug.abiO64)) }
  .mdebug.eabi32 : { KEEP(*(.mdebug.eabi32)) }
  .mdebug.eabi64 : { KEEP(*(.mdebug.eabi64)) }
  .gcc_compiled_long32 : { KEEP(*(.gcc_compiled_long32)) }
  .gcc_compiled_long64 : { KEEP(*(.gcc_compiled_long64)) }
  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) }
}
