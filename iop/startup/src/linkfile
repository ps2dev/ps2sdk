/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Linkfile script for iop-ld
*/

OUTPUT_FORMAT("elf32-littlemips", "elf32-bigmips",
        "elf32-littlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
 SEARCH_DIR("");
/* FORCE_COMMON_ALLOCATION */
/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = 0x0400000; /* Can conditionally be changed to . = 0x5ffe0000 + SIZEOF_HEADERS; */
  .interp     : { *(.interp)  } /* Can conditionally be removed */
  .reginfo ALIGN(16) : SUBALIGN(16) { *(.reginfo) }
  .hash          : { *(.hash)   }
  .dynsym        : { *(.dynsym)   }
  .dynstr        : { *(.dynstr)   }
  .gnu.version   : { *(.gnu.version)  }
  .gnu.version_d   : { *(.gnu.version_d)  }
  .gnu.version_r   : { *(.gnu.version_r)  }
  .rel.text      :
    { *(.rel.text) *(.rel.gnu.linkonce.t*) }
  .rela.text     :
    { *(.rela.text) *(.rela.gnu.linkonce.t*) }
  .rel.data      :
    { *(.rel.data) *(.rel.gnu.linkonce.d*) }
  .rela.data     :
    { *(.rela.data) *(.rela.gnu.linkonce.d*) }
  .rel.rodata    :
    { *(.rel.rodata) *(.rel.gnu.linkonce.r*) }
  .rela.rodata   :
    { *(.rela.rodata) *(.rela.gnu.linkonce.r*) }
  .rel.got       : { *(.rel.got)    }
  .rela.got      : { *(.rela.got)   }
  .rel.ctors     : { *(.rel.ctors)  }
  .rela.ctors    : { *(.rela.ctors) }
  .rel.dtors     : { *(.rel.dtors)  }
  .rela.dtors    : { *(.rela.dtors) }
  .rel.init      : { *(.rel.init) }
  .rela.init     : { *(.rela.init)  }
  .rel.fini      : { *(.rel.fini) }
  .rela.fini     : { *(.rela.fini)  }
  .rel.bss       : { *(.rel.bss)    }
  .rela.bss      : { *(.rela.bss)   }
  .rel.plt       : { *(.rel.plt)    }
  .rela.plt      : { *(.rela.plt)   }
  .init          : { *(.init) } =0
  .plt      : { *(.plt) }
  .text ALIGN(16) : SUBALIGN(16)
  {
    PROVIDE(_ftext = . );
    *(.text)
    *(.stub)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.gnu.linkonce.t*)
    *(.mips16.fn.*) *(.mips16.call.*)
  } =0
  PROVIDE(_etext = .);
  PROVIDE (etext = .);
  .fini      : { *(.fini)    } =0
  .rodata ALIGN(16) : SUBALIGN(16) { *(.rodata) *(.rodata.*) *(.gnu.linkonce.r*) }
  .rodata1   : { *(.rodata1) }
  /* Adjust the address for the data segment.  We want to adjust up to
     the same address within the page on the next page up.  */
  . = ALIGN(0x40000) + (. & (0x40000 - 1)); /* Can conditionally be changed to . = .; */
  .data ALIGN(16) : SUBALIGN(16)
  {
    PROVIDE(_fdata = .);
    *(.data)
    *(.gnu.linkonce.d*)
    CONSTRUCTORS
  }
  .data1   : { *(.data1) }
  .ctors         :
  {
    *(.ctors)
  }
  .dtors         :
  {
    *(.dtors)
  }
  PROVIDE(_gp = ALIGN(16) + 0x7ff0);
  .got           : { *(.got.plt) *(.got) }
  .dynamic       : { *(.dynamic) }
  /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
  .sdata     : { *(.sdata) }
  .lit8 : { *(.lit8) }
  .lit4 : { *(.lit4) }
  PROVIDE(_edata  =  .);
  PROVIDE (edata = .);
  __bss_start = .;
  PROVIDE(_fbss = .);
  .sbss      : { *(.sbss) *(.scommon) }
  .bss ALIGN(16) : SUBALIGN(16)
  {
   *(.dynbss)
   *(.bss)
   *(COMMON)
  }
  . = ALIGN(32 / 8);
  PROVIDE(_end = .);
  PROVIDE (end = .);
  /* Stabs debugging sections.  */
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment 0 : { *(.comment) }
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
  .debug_info     0 : { *(.debug_info) }
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
  /* These must appear regardless of  .  */
  .gptab.sdata : { *(.gptab.data) *(.gptab.sdata) }
  .gptab.sbss : { *(.gptab.bss) *(.gptab.sbss) }
  /*
   * These are the stuff that we don't want to be put in an IRX.
   */
  /DISCARD/ : {
    * ( .MIPS.abiflags )
  }
}
