/* -*- Makefile -*- */
ENTRY(_start)

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
  L4_DEFINE_X86_KERNEL_ENTRY_SYMS
  PROVIDE (__executable_start = 0x0100000);
  . = __executable_start + SIZEOF_HEADERS;

  /* No program code/data before _stext/_prog_img_start! */

  /* Read-only sections, merged into text segment. The start address of
   * the text segment is : */
  /DISCARD/ :
  { *(.note.gnu.build-id)
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
  *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*)
  *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
  *(.rel.plt)
  *(.rela.plt)
  *(.stapsdt.*)
  *(.note.stapsdt)
  *(.plt) }
  .text :
  {
    _stext = .;
    _prog_img_start = _stext;
    *(.text .stub .text.* .gnu.linkonce.t.*)
    KEEP (*(.text.*personality*))
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
  } :ro =0x90909090
  .init           :
  {
    KEEP (*(.init))
  } :ro =0x90909090
  .fini           :
  {
    KEEP (*(.fini))
    /* LONG(0xc3)  * terminate .fini */
  } :ro = 0x9090

  PROVIDE (_etext = .);
  PROVIDE (etext = .);
  .rol4re_elf_aux : {
    KEEP (*(.rol4re_elf_aux))
  } : l4re_aux : ro
  .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) } : ro
  .rodata1        : { *(.rodata1) }
  .eh_frame_hdr : { *(.eh_frame_hdr) } :ro

  .eh_frame       : ONLY_IF_RO
  {
    PROVIDE(__eh_frame_start__ = .);
    KEEP (*(.eh_frame))
    KEEP (*(.eh_frame_terminator))
  } :ro

  .gcc_except_table   : ONLY_IF_RO { *(.gcc_except_table .gcc_except_table.*) }


  . = ALIGN(CONSTANT(MAXPAGESIZE));
  /* Adjust the address for the data segment.  We want to adjust up to
     the same address within the page on the next page up.  */
  . = ALIGN (CONSTANT (MAXPAGESIZE)) - ((CONSTANT (MAXPAGESIZE) - .) & (CONSTANT (MAXPAGESIZE) - 1)); . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
  PROVIDE (_sdata = .);
  /* Exception handling  */
  .eh_frame       : ONLY_IF_RW
  {
    PROVIDE (__eh_frame_start__ = .);
    KEEP (*(.eh_frame))
    KEEP (*(.eh_frame_terminator))
  } :rw
  .gcc_except_table   : ONLY_IF_RW { *(.gcc_except_table .gcc_except_table.*) } :rw
  /* Thread Local Storage sections  */
  .tdata	  : { *(.tdata .tdata.* .gnu.linkonce.td.*) } : rw : tls
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
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }
  /* ensure that data starts at a new L4 page */
  /*. = ALIGN(0x1000);*/
  .ctors :
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
    /*
    KEEP (*(.mark_end_ctors))

    KEEP (*(.mark_beg_c_ctors))
    KEEP (*(SORT(.c_ctors.?)))
    KEEP (*(SORT(.c_ctors.??)))
    KEEP (*(SORT(.c_ctors.???)))
    KEEP (*(SORT(.c_ctors.????)))
    KEEP (*(SORT(.c_ctors.?????)))
    KEEP (*(.c_ctors))
    KEEP (*(.mark_end_c_ctors))
    */
  } :rw

  .dtors :
  {
  /*
    KEEP (*(.mark_beg_dtors))*/
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    /*
    KEEP (*(.mark_end_dtors))

    KEEP (*(.mark_beg_c_sys_dtors))
    KEEP (*(SORT(.c_dtors.?)))
    KEEP (*(SORT(.c_dtors.??)))
    KEEP (*(.mark_end_c_sys_dtors))

    KEEP (*(.mark_beg_c_dtors))
    KEEP (*(SORT(.c_dtors.???)))
    KEEP (*(SORT(.c_dtors.????)))
    KEEP (*(SORT(.c_dtors.?????)))
    KEEP (*(.c_dtors))
    KEEP (*(.mark_end_c_dtors))*/
  }
  .data           :
  {
    PROVIDE_HIDDEN(__l4sys_invoke_indirect = .);
    LONG (0xeacff000) /*__l4sys_invoke_direct)*/
    *(.data .data.* .gnu.linkonce.d.*)
    KEEP (*(.gnu.linkonce.d.*personality*))
    SORT(CONSTRUCTORS)
  }
  .data1          : { *(.data1) }
  _edata = .; PROVIDE (edata = .);
  __bss_start = .;
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
   . = ALIGN(. != 0 ? 32 / 8 : 1);
  }
  . = ALIGN(32 / 8);
  . = ALIGN(32 / 8);
  _end = .; PROVIDE (end = .);
  . = DATA_SEGMENT_END (.);
  . = ALIGN(CONSTANT(MAXPAGESIZE));
  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) } :NONE
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
  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.interp) }
}
