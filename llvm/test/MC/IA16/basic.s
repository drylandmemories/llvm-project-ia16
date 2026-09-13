# RUN: llvm-mc -triple=ia16-unknown-none-elf -mcpu=i8086 -show-encoding %s | FileCheck %s
# RUN: llvm-mc -triple=ia16-pc-dos-elf -mcpu=i8086 -filetype=obj %s -o %t
# RUN: llvm-readobj -h %t | FileCheck %s --check-prefix=ELF

# CHECK: movw (%bx,%si), %ax # encoding: [0x8b,0x00]
# CHECK: addw %dx, %ax # encoding: [0x01,0xd0]
# CHECK: retw # encoding: [0xc3]

# ELF: Class: 32-bit
# ELF: DataEncoding: LittleEndian
# ELF: Machine: EM_386 (0x3)

movw (%bx,%si), %ax
addw %dx, %ax
retw
