# REQUIRES: x86

# RUN: llvm-mc -triple=i386 -filetype=obj %s -o %t.o
# RUN: llvm-readobj -r %t.o | FileCheck %s --check-prefix=RELOCS
# RUN: ld.lld -m elf_i386 --entry=0 --defsym=zero=0 --defsym=one=1 \
# RUN:   --defsym=negative_one=0xffffffff --defsym=zero_target=0x80000000 \
# RUN:   --defsym=positive_target=0x80000001 \
# RUN:   --defsym=negative_target=0x7fffffff -o %t %t.o
# RUN: llvm-objdump -s -j .data %t | FileCheck %s --check-prefix=MIN
# RUN: not ld.lld -m elf_i386 --entry=0 --defsym=zero=0 --defsym=one=1 \
# RUN:   --defsym=negative_one=0xffffffff --defsym=zero_target=0x80000001 \
# RUN:   --defsym=positive_target=0x80000001 \
# RUN:   --defsym=negative_target=0x7fffffff -o /dev/null %t.o 2>&1 \
# RUN:   | FileCheck %s --check-prefix=ZERO-BELOW-MIN
# RUN: not ld.lld -m elf_i386 --entry=0 --defsym=zero=0 --defsym=one=1 \
# RUN:   --defsym=negative_one=0xffffffff --defsym=zero_target=0x80000000 \
# RUN:   --defsym=positive_target=0x80000002 \
# RUN:   --defsym=negative_target=0x7fffffff -o /dev/null %t.o 2>&1 \
# RUN:   | FileCheck %s --check-prefix=POSITIVE-BELOW-MIN
# RUN: not ld.lld -m elf_i386 --entry=0 --defsym=zero=0 --defsym=one=1 \
# RUN:   --defsym=negative_one=0xffffffff --defsym=zero_target=0x80000000 \
# RUN:   --defsym=positive_target=0x80000001 \
# RUN:   --defsym=negative_target=0x80000000 -o /dev/null %t.o 2>&1 \
# RUN:   | FileCheck %s --check-prefix=NEGATIVE-BELOW-MIN

# RELOCS: R_386_32 zero
# RELOCS-NEXT: R_386_SUB32 zero_target
# RELOCS-NEXT: R_386_32 one
# RELOCS-NEXT: R_386_SUB32 positive_target
# RELOCS-NEXT: R_386_32 negative_one
# RELOCS-NEXT: R_386_SUB32 negative_target

# Same-offset relocations are cumulative. Each passing expression below is
# exactly -2147483648, with zero, positive, and negative cumulative addends.
# MIN: Contents of section .data:
# MIN: 00000080 00000080 00000080

# ZERO-BELOW-MIN: error: {{.*}}:(.data+0x0): relocation R_386_SUB32 out of range: -2147483649 is not in [-2147483648, 2147483647]; references 'zero_target'
# POSITIVE-BELOW-MIN: error: {{.*}}:(.data+0x4): relocation R_386_SUB32 out of range: -2147483649 is not in [-2147483648, 2147483647]; references 'positive_target'
# NEGATIVE-BELOW-MIN: error: {{.*}}:(.data+0x8): relocation R_386_SUB32 out of range: -2147483649 is not in [-2147483648, 2147483647]; references 'negative_target'

.section .data,"aw",@progbits
.long 0
.reloc .-4, R_386_32, zero
.reloc .-4, R_386_SUB32, zero_target
.long 0
.reloc .-4, R_386_32, one
.reloc .-4, R_386_SUB32, positive_target
.long 0
.reloc .-4, R_386_32, negative_one
.reloc .-4, R_386_SUB32, negative_target
