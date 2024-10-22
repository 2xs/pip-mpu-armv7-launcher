source symbols.gdb
set architecture arm
target extended-remote :3333
monitor arm semihosting enable
