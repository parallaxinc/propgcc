cat <<EOF
OUTPUT_FORMAT("${OUTPUT_FORMAT}","${OUTPUT_FORMAT}","${OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})

MEMORY
{
  ${P2-"hub     : ORIGIN = 0, LENGTH = 32K"}
  ${P2+"hub     : ORIGIN = 0, LENGTH = 128K"}
  cog	  : ORIGIN = 0, LENGTH = 1984 /* 496*4 */
  /* coguser is just an alias for cog, but for overlays */
  coguser : ORIGIN = 0, LENGTH = 1984 /* 496*4 */
  /* kernel memory is where the .lmm or .xmm kernel goes */
  kermem  : ORIGIN = 0, LENGTH = 0x6C0
  kerextmem : ORIGIN = 0x6C0, LENGTH = 0x100
  /* bootpasm is an alias for kernel */
  bootpasm : ORIGIN = 0, LENGTH = 0x6C0

  ram     : ORIGIN = 0x20000000, LENGTH = 256M
  rom     : ORIGIN = 0x30000000, LENGTH = 256M
  /* some sections (like the .xmm kernel) are handled specially by the loader */
  drivers : ORIGIN = 0xc0000000, LENGTH = 1M
  dummy   : ORIGIN = 0xe0000000, LENGTH = 1M
}

SECTIONS
{
  /* if we are not relocating (-r flag given) then discard the boot and bootpasm sections; otherwise keep them */
  ${RELOCATING- "/DISCARD/ : \{ *(.boot) \}" }
  ${RELOCATING- "/DISCARD/ : \{ *(.bootpasm) \}" }

  /* the initial spin boot code, if any */
  ${RELOCATING+ ".boot : \{ KEEP(*(.boot)) \} >hub" }
  ${RELOCATING+ ".bootpasm : \{ KEEP(*(.bootpasm)) \} >bootpasm AT>hub" }

  /* the initial startup code (including constructors) */
  .init ${RELOCATING-0} :
  {
    KEEP(*(.init*))
  } ${RELOCATING+ ${TEXT_MEMORY}}

  /* Internal text space or external memory.  */
  .text ${RELOCATING-0} :
  {
    *(.text*)
    ${RELOCATING+ _etext = . ; }
  } ${RELOCATING+ ${TEXT_MEMORY}}

  /* the final cleanup code (including destructors) */
  .fini ${RELOCATING-0} :
  {
    *(.fini*)
  } ${RELOCATING+ ${TEXT_MEMORY}}

  .hub ${RELOCATING-0} :
  {
    *(.hubstart)
    *(.hubtext*)
    *(.hubdata*)
    *(.hub)
    ${HUB_DATA}
    ${RELOCATING+ PROVIDE(__C_LOCK = .); LONG(0); }
  } ${RELOCATING+ ${HUBTEXT_MEMORY}}
  ${TEXT_DYNAMIC+${DYNAMIC}}

  .ctors ${RELOCATING-0} :
  {
    KEEP(*(.ctors*))
  } ${RELOCATING+ ${HUBTEXT_MEMORY}}

  .dtors ${RELOCATING-0} :
  {
    KEEP(*(.dtors*))
  } ${RELOCATING+ ${HUBTEXT_MEMORY}}

  .data	${RELOCATING-0} :
  {
    ${DATA_DATA}
    ${RELOCATING+. = ALIGN(4);}
  } ${RELOCATING+ ${DATA_MEMORY}}

  ${KERNEL}

  .bss ${RELOCATING-0} ${RELOCATING+ $BSS_OVERLAY} :
  {
    ${RELOCATING+ PROVIDE (__bss_start = .) ; }
    *(.bss)
    *(.bss*)
    *(COMMON)
    ${RELOCATING+ PROVIDE (__bss_end = .) ; }
  } ${RELOCATING+ ${DATA_MEMORY}}

  ${RELOCATING+ ${DATA_HEAP+ ".heap : \{ . += 4; \} ${DATA_MEMORY}"}}
  ${RELOCATING+ ${DATA_HEAP+ ___heap_start = ADDR(.heap) ;}}
  ${RELOCATING+ ${HUB_HEAP+ ".hub_heap : \{ . += 4; \} >hub AT>hub"}}
  ${RELOCATING+ ${HUB_HEAP+ ___hub_heap_start = ADDR(.hub_heap) ;}}

  .drivers ${RELOCATING-0} :
  {
    *(.drivers)
    /* the linker will place .ecog sections after this section */
  } ${RELOCATING+ AT>drivers}

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

  /* global variables */
  ${P2-"PROVIDE(__clkfreq = 0x0) ;"}
  ${P2-"PROVIDE(__clkmode = 0x4) ;"}

  /* provide some register definitions - propeller 1 */
  ${P2-"PROVIDE(PAR = 0x7C0) ;"}
  ${P2-"PROVIDE(CNT = 0x7C4) ;"}
  ${P2-"PROVIDE(INA = 0x7C8) ;"}
  ${P2-"PROVIDE(INB = 0x7CC) ;"}
  ${P2-"PROVIDE(OUTA = 0x7D0) ;"}
  ${P2-"PROVIDE(OUTB = 0x7D4) ;"}
  ${P2-"PROVIDE(DIRA = 0x7D8) ;"}
  ${P2-"PROVIDE(DIRB = 0x7DC) ;"}
  ${P2-"PROVIDE(CTRA = 0x7E0) ;"}
  ${P2-"PROVIDE(CTRB = 0x7E4) ;"}
  ${P2-"PROVIDE(FRQA = 0x7E8) ;"}
  ${P2-"PROVIDE(FRQB = 0x7EC) ;"}
  ${P2-"PROVIDE(PHSA = 0x7F0) ;"}
  ${P2-"PROVIDE(PHSB = 0x7F4) ;"}
  ${P2-"PROVIDE(VCFG = 0x7F8) ;"}
  ${P2-"PROVIDE(VSCL = 0x7FC) ;"}

  /* provide some case-sensitive aliases - propeller 1 */
  ${P2-"PROVIDE(par = PAR) ;"}
  ${P2-"PROVIDE(cnt = CNT) ;"}
  ${P2-"PROVIDE(ina = INA) ;"}
  ${P2-"PROVIDE(inb = INB) ;"}
  ${P2-"PROVIDE(outa = OUTA) ;"}
  ${P2-"PROVIDE(outb = OUTB) ;"}
  ${P2-"PROVIDE(dira = DIRA) ;"}
  ${P2-"PROVIDE(dirb = DIRB) ;"}
  ${P2-"PROVIDE(ctra = CTRA) ;"}
  ${P2-"PROVIDE(ctrb = CTRB) ;"}
  ${P2-"PROVIDE(frqa = FRQA) ;"}
  ${P2-"PROVIDE(frqb = FRQB) ;"}
  ${P2-"PROVIDE(phsa = PHSA) ;"}
  ${P2-"PROVIDE(phsb = PHSB) ;"}
  ${P2-"PROVIDE(vcfg = VCFG) ;"}
  ${P2-"PROVIDE(vscl = VSCL) ;"}

  /* provide some case-sensitive aliases - propeller 2 */
  ${P2+"PROVIDE(pina = PINA) ;"}
  ${P2+"PROVIDE(pinb = PINB) ;"}
  ${P2+"PROVIDE(pinc = PINC) ;"}
  ${P2+"PROVIDE(pind = PIND) ;"}
  ${P2+"PROVIDE(dira = DIRA) ;"}
  ${P2+"PROVIDE(dirb = DIRB) ;"}
  ${P2+"PROVIDE(dirc = DIRC) ;"}
  ${P2+"PROVIDE(dird = DIRD) ;"}

  /* this symbol is used to tell the spin boot code where the spin stack can go */
  ${RELOCATING+ "PROVIDE(__hub_end = ADDR(.hub_heap) + 16) ;"}
  
  /* default initial stack pointer */
  ${P2-"PROVIDE(__stack_end = 0x8000) ;"}
  ${P2+"PROVIDE(__stack_end = 0x20000) ;"}

}
EOF

