# REQUIRES: x86

# RUN: llvm-mc -triple=i386 -filetype=obj %s -o %t.o
# RUN: llvm-readobj -r %t.o | FileCheck %s --check-prefix=RELOCS
# RUN: ld.lld -m elf_i386 --entry=0 --defsym=target=0x80000000 \
# RUN:   --defsym=negative_addend_target=0x7fffffff -o %t %t.o
# RUN: llvm-objdump -s -j .data %t | FileCheck %s --check-prefix=MIN
# RUN: not ld.lld -m elf_i386 --entry=0 --defsym=target=0x80000001 \
# RUN:   --defsym=negative_addend_target=0x7fffffff -o /dev/null %t.o 2>&1 \
# RUN:   | FileCheck %s --check-prefix=BELOW-MIN
# RUN: not ld.lld -m elf_i386 --entry=0 --defsym=target=0x80000000 \
# RUN:   --defsym=negative_addend_target=0x80000000 -o /dev/null %t.o 2>&1 \
# RUN:   | FileCheck %s --check-prefix=NEGATIVE-ADDEND

# RELOCS: R_386_SUB32 target
# RELOCS-NEXT: R_386_SUB32 negative_addend_target

# A-S must fit signed 32 bits even when S has its high bit set. Exercise both
# zero and negative implicit addends, including S+A crossing the signed boundary.
# MIN: Contents of section .data:
# MIN: 00000080 00000080

# BELOW-MIN: error: {{.*}}:(.data+0x0): relocation R_386_SUB32 out of range: -2147483649 is not in [-2147483648, 2147483647]; references 'target'
# NEGATIVE-ADDEND: error: {{.*}}:(.data+0x4): relocation R_386_SUB32 out of range: -2147483649 is not in [-2147483648, 2147483647]; references 'negative_addend_target'

.section .data,"a",@progbits
.long 0
.reloc .-4, R_386_SUB32, target
.long -1
.reloc .-4, R_386_SUB32, negative_addend_target
