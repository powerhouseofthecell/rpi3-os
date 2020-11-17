SECTIONS
{
    . = 0x80000;

    .text : {
        KEEP(*(.text.boot))
        *(.text .text.* .gnu.linkonce.t*)
    }

    .rodata : {
        *(.rodata .rodata.* .gnu.linkonce.r*)
    }

    /* Constructors: these sections support global initialization
       functions, such as for global C++ objects with constructors. */
    .init_array : {
        PROVIDE(__init_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*)
                SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP (*(.init_array .ctors))
        PROVIDE(__init_array_end = .);
    }
    .ctors : {
        KEEP (*crtbegin.o(.ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
    }

    . = ALIGN(4096);
    PROVIDE(_data = .);
    .data : {
        *(.data .data.* .gnu.linkonce.d*)
    }

    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    
    . = ALIGN(4096);
    PROVIDE(_kernel_end = .);

   /DISCARD/ : {
       *(.comment) *(.gnu*) *(.note*) *(.eh_frame*)
    }
}
__bss_size = (__bss_end - __bss_start);
