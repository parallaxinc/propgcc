cat <<EOF
OUTPUT_FORMAT("${OUTPUT_FORMAT}","${OUTPUT_FORMAT}","${OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})

MEMORY
{
  hub    : ORIGIN = 0, LENGTH = 32K
  cog	 : ORIGIN = 0, LENGTH = 2K
  /* coguser is just an alias for cog, but for overlays */
  coguser : ORIGIN = 0, LENGTH = 2K
  rom    : ORIGIN = 0x30000000, LENGTH = 1M
}

SECTIONS
{
  /* the initial spin boot code, if any */
  .boot : { *(.boot) } ${RELOCATING+ >hub}

  ${KERNEL}
  ${XMM_HEADER}

  /* the initial startup code (including constructors) */
  .init ${RELOCATING-0} :
  {
    *(.init*)
  } ${RELOCATING+ ${TEXT_MEMORY}}

  /* Internal text space or external memory.  */
  .text ${RELOCATING-0} :
  {
    *(EXCLUDE_FILE (*.cog) .text*)
    ${RELOCATING+ _etext = . ; }
  } ${RELOCATING+ ${TEXT_MEMORY}}

  .hubtext ${RELOCATING-0} :
  {
    *(.hubtext*)
  } ${RELOCATING+ ${HUBTEXT_MEMORY}}
  ${TEXT_DYNAMIC+${DYNAMIC}}

  .data	${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__data_start = .) ; }
    *(.data)
    *(.data*)
    *(.rodata)  /* We need to include .rodata here if gcc is used */
    *(.rodata*) /* with -fdata-sections.  */
    *(.gnu.linkonce.d*)
    ${RELOCATING+. = ALIGN(4);}
    ${RELOCATING+ _edata = . ; }
    ${RELOCATING+ PROVIDE (__data_end = .) ; }
  } ${RELOCATING+ ${DATA_MEMORY}}

  .ctors ${RELOCATING-0} :
  {
    *(.ctors*)
  } ${RELOCATING+ ${DATA_MEMORY}}
  .dtors ${RELOCATING-0} :
  {
    *(.dtors*)
  } ${RELOCATING+ ${DATA_MEMORY}}

  .bss ${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__bss_start = .) ; }
    *(.bss)
    *(.bss*)
    *(COMMON)
    ${RELOCATING+ PROVIDE (__bss_end = .) ; }
  } ${RELOCATING+ > hub}

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

  } ${RELOCATING+ >coguser AT>hub}

  ${RELOCATING+ ".heap : \{ LONG(0) \} > hub"}
  ${RELOCATING+ ___heap_start = ADDR(.heap) ;}

  ${RELOCATING+ ${KERNEL_NAME+ __load_start_kernel = LOADADDR (${KERNEL_NAME}) ;}}
  ${RELOCATING+ ___CTOR_LIST__ = ADDR(.ctors) ;}
  ${RELOCATING+ ___DTOR_LIST__ = ADDR(.dtors) ;}

  .hash        ${RELOCATING-0} : { *(.hash)		}
  .dynsym      ${RELOCATING-0} : { *(.dynsym)		}
  .dynstr      ${RELOCATING-0} : { *(.dynstr)		}
  .gnu.version ${RELOCATING-0} : { *(.gnu.version)	}
  .gnu.version_d ${RELOCATING-0} : { *(.gnu.version_d)	}
  .gnu.version_r ${RELOCATING-0} : { *(.gnu.version_r)	}

  .rel.init    ${RELOCATING-0} : { *(.rel.init)		}
  .rela.init   ${RELOCATING-0} : { *(.rela.init)	}
  .rel.text    ${RELOCATING-0} :
    {
      *(.rel.text)
      ${RELOCATING+*(.rel.text.*)}
      ${RELOCATING+*(.rel.gnu.linkonce.t*)}
    }
  .rela.text   ${RELOCATING-0} :
    {
      *(.rela.text)
      ${RELOCATING+*(.rela.text.*)}
      ${RELOCATING+*(.rela.gnu.linkonce.t*)}
    }
  .rel.fini    ${RELOCATING-0} : { *(.rel.fini)		}
  .rela.fini   ${RELOCATING-0} : { *(.rela.fini)	}
  .rel.rodata  ${RELOCATING-0} :
    {
      *(.rel.rodata)
      ${RELOCATING+*(.rel.rodata.*)}
      ${RELOCATING+*(.rel.gnu.linkonce.r*)}
    }
  .rela.rodata ${RELOCATING-0} :
    {
      *(.rela.rodata)
      ${RELOCATING+*(.rela.rodata.*)}
      ${RELOCATING+*(.rela.gnu.linkonce.r*)}
    }
  .rel.data    ${RELOCATING-0} :
    {
      *(.rel.data)
      ${RELOCATING+*(.rel.data.*)}
      ${RELOCATING+*(.rel.gnu.linkonce.d*)}
    }
  .rela.data   ${RELOCATING-0} :
    {
      *(.rela.data)
      ${RELOCATING+*(.rela.data.*)}
      ${RELOCATING+*(.rela.gnu.linkonce.d*)}
    }
  .rel.ctors   ${RELOCATING-0} : { *(.rel.ctors)	}
  .rela.ctors  ${RELOCATING-0} : { *(.rela.ctors)	}
  .rel.dtors   ${RELOCATING-0} : { *(.rel.dtors)	}
  .rela.dtors  ${RELOCATING-0} : { *(.rela.dtors)	}
  .rel.got     ${RELOCATING-0} : { *(.rel.got)		}
  .rela.got    ${RELOCATING-0} : { *(.rela.got)		}
  .rel.bss     ${RELOCATING-0} : { *(.rel.bss)		}
  .rela.bss    ${RELOCATING-0} : { *(.rela.bss)		}
  .rel.plt     ${RELOCATING-0} : { *(.rel.plt)		}
  .rela.plt    ${RELOCATING-0} : { *(.rela.plt)		}

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
  .debug_info     0 : { *(.debug_info${RELOCATING+ .gnu.linkonce.wi.*} .zdebug_info) }
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
EOF

