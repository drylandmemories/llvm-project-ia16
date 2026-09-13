# REQUIRES: x86

# RUN: llvm-mc -triple=i386 -filetype=obj %s -o %t.o
# RUN: llvm-readobj -r %t.o | FileCheck %s --check-prefix=RELOCS
# RUN: ld.lld -m elf_i386 --entry=0 --defsym=target=0x8000 -o %t %t.o
# RUN: llvm-objdump -s -j .data %t | FileCheck %s --check-prefix=MIN
# RUN: not ld.lld -m elf_i386 --entry=0 --defsym=target=0x8001 \
# RUN:   -o /dev/null %t.o 2>&1 | FileCheck %s --check-prefix=BELOW-MIN

# RELOCS: R_386_SUB16 target

# MIN: Contents of section .data:
# MIN: 0080

# BELOW-MIN: error: {{.*}}:(.data+0x0): relocation R_386_SUB16 out of range: -32769 is not in [-32768, 32767]; references 'target'

.section .data,"a",@progbits
.short 0
.reloc .-2, R_386_SUB16, target
