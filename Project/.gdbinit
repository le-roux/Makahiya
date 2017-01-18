target ext:2331
mon endian little
mon halt

# Command to get a proper gdb display
define split
    layout split
    layout asm
    layout regs
    focus cmd
end
