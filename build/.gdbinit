target remote localhost:1234

tui enable
layout src
layout regs

b kernel_main
b irq_wrapper

display/x $sp
display/x $sp_el0
display/x $sp_el1
display/x $spsel
