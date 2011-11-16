OUTPUT_FORMAT("elf32-propeller","elf32-propeller","elf32-propeller")
OUTPUT_ARCH(propeller)
MEMORY
{
  cog	  : ORIGIN = 0, LENGTH = 1984 /* 496*4 */
  /* coguser is just an alias for cog, but for overlays */
  coguser : ORIGIN = 0, LENGTH = 1984 /* 496*4 */
  /* some sections (like the .xmm kernel) are handled specially by the loader */
  dummy   : ORIGIN = 0xe0000000, LENGTH = 1M
}
SECTIONS
{
  /* the initial spin boot code, if any */
  .boot : { *(.boot) }  >hub
  /* the LMM kernel that is loaded into the cog */
  .xmmkernel   :
  {
    *(.xmmkernel) *(.kernel)
  } >cog AT>dummy
    .header : {
        LONG(entry)
        LONG(0)
        LONG(0)
    } >REGION_TEXT
  /* the initial startup code (including constructors) */
  .init   :
  {
    *(.init*)
  }  >REGION_TEXT
  /* Internal text space or external memory.  */
  .text   :
  {
    *(EXCLUDE_FILE (*.cog) .text*)
     _etext = . ;
  }  >REGION_TEXT
  /* the final cleanup code (including destructors) */
  .fini   :
  {
    *(.fini*)
  }  >REGION_TEXT
  .hub   :
  {
    *(.hubstart)
    *(.hubtext*)
  }  >hub AT>REGION_TEXT
  /* this needs to be after .hub in case data is in hub memory */
  .data	  :
  {
    *(.data)
    *(.data*)
    *(.rodata)  /* We need to include .rodata here if gcc is used */
    *(.rodata*) /* with -fdata-sections.  */
    *(.gnu.linkonce.d*)
    . = ALIGN(4);
  }  >REGION_DATA AT>REGION_TEXT
  .ctors   :
  {
    *(.ctors*)
  }  >hub AT>REGION_TEXT
  .dtors   :
  {
    *(.dtors*)
  }  >hub AT>REGION_TEXT
  .bss   :
  {
     PROVIDE (__bss_start = .) ;
    *(.bss)
    *(.bss*)
    *(COMMON)
     PROVIDE (__bss_end = .) ;
  }  >REGION_DATA
  /* put the cog drivers after bss and just before the heap */
  /* that way we may later be able to free the hub memory they take up */
  OVERLAY : {
      .cogsys0 { *(.cogsys0) }
      .cogsys1 { *(.cogsys1) }
      .cogsys2 { *(.cogsys2) }
      .cogsys3 { *(.cogsys3) }
      .cogsys4 { *(.cogsys4) }
      .cogsys5 { *(.cogsys5) }
      .cogsys6 { *(.cogsys6) }
      .cogsys7 { *(.cogsys7) }
      .coguser0 { *(.coguser0) *0.cog(.text*) }
      .coguser1 { *(.coguser1) *1.cog(.text*) }
      .coguser2 { *(.coguser2) *2.cog(.text*) }
      .coguser3 { *(.coguser3) *3.cog(.text*) }
      .coguser4 { *(.coguser4) *4.cog(.text*) }
      .coguser5 { *(.coguser5) *5.cog(.text*) }
      .coguser6 { *(.coguser6) *6.cog(.text*) }
      .coguser7 { *(.coguser7) *7.cog(.text*) }
  }  >coguser AT>REGION_TEXT
   .heap : { LONG(0) } >REGION_HEAP AT>REGION_HEAP
   ___heap_start = ADDR(.heap) ;
    __load_start_kernel = LOADADDR (.xmmkernel) ;
   ___CTOR_LIST__ = ADDR(.ctors) ;
   ___DTOR_LIST__ = ADDR(.dtors) ;
  .hash          : { *(.hash)		}
  .dynsym        : { *(.dynsym)		}
  .dynstr        : { *(.dynstr)		}
  .gnu.version   : { *(.gnu.version)	}
  .gnu.version_d   : { *(.gnu.version_d)	}
  .gnu.version_r   : { *(.gnu.version_r)	}
  .rel.init      : { *(.rel.init)		}
  .rela.init     : { *(.rela.init)	}
  .rel.text      :
    {
      *(.rel.text)
      *(.rel.text.*)
      *(.rel.gnu.linkonce.t*)
    }
  .rela.text     :
    {
      *(.rela.text)
      *(.rela.text.*)
      *(.rela.gnu.linkonce.t*)
    }
  .rel.fini      : { *(.rel.fini)		}
  .rela.fini     : { *(.rela.fini)	}
  .rel.rodata    :
    {
      *(.rel.rodata)
      *(.rel.rodata.*)
      *(.rel.gnu.linkonce.r*)
    }
  .rela.rodata   :
    {
      *(.rela.rodata)
      *(.rela.rodata.*)
      *(.rela.gnu.linkonce.r*)
    }
  .rel.data      :
    {
      *(.rel.data)
      *(.rel.data.*)
      *(.rel.gnu.linkonce.d*)
    }
  .rela.data     :
    {
      *(.rela.data)
      *(.rela.data.*)
      *(.rela.gnu.linkonce.d*)
    }
  .rel.ctors     : { *(.rel.ctors)	}
  .rela.ctors    : { *(.rela.ctors)	}
  .rel.dtors     : { *(.rel.dtors)	}
  .rela.dtors    : { *(.rela.dtors)	}
  .rel.got       : { *(.rel.got)		}
  .rela.got      : { *(.rela.got)		}
  .rel.bss       : { *(.rel.bss)		}
  .rela.bss      : { *(.rela.bss)		}
  .rel.plt       : { *(.rel.plt)		}
  .rela.plt      : { *(.rela.plt)		}
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
  .debug_srcinfo  0 : { *(.debug_srcinfo .zdebug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames .zdebug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges .zdebug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames .zdebug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.* .zdebug_info) }
  .debug_abbrev   0 : { *(.debug_abbrev .zdebug_abbrev) }
  .debug_line     0 : { *(.debug_line .zdebug_line) }
  .debug_frame    0 : { *(.debug_frame .zdebug_frame) }
  .debug_str      0 : { *(.debug_str .zdebug_str) }
  .debug_loc      0 : { *(.debug_loc .zdebug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo .zdebug_macinfo) }
  /* provide some case-sensitive aliases */
  PROVIDE(par = PAR) ;
  PROVIDE(cnt = CNT) ;
  PROVIDE(dira = DIRA) ;
  PROVIDE(outa = OUTA) ;
  PROVIDE(ina = INA) ;
}
