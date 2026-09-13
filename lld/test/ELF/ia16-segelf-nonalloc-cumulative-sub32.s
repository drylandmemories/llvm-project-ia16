# REQUIRES: x86

# RUN: llvm-mc -triple=i386 -filetype=obj %s -o %t.o
# RUN: llvm-readobj --section-headers --relocations %t.o | \
# RUN:   FileCheck %s --check-prefix=OBJECT
# RUN: ld.lld -m elf_i386 --entry=0 --defsym=first=1 --defsym=second=2 \
# RUN:   --defsym=zero=0 --defsym=one=1 --defsym=negative_one=0xffffffff \
# RUN:   --defsym=zero_target=0x80000000 \
# RUN:   --defsym=positive_target=0x80000001 \
# RUN:   --defsym=negative_target=0x7fffffff --emit-relocs -o %t %t.o
# RUN: llvm-objdump -s -j .data -j .debug_ia16 %t | \
# RUN:   FileCheck %s --check-prefix=CONTENTS
# RUN: llvm-readobj --relocations %t | FileCheck %s --check-prefix=RETAINED
# RUN: not ld.lld -m elf_i386 --entry=0 --defsym=first=1 --defsym=second=2 \
# RUN:   --defsym=zero=0 --defsym=one=1 --defsym=negative_one=0xffffffff \
# RUN:   --defsym=zero_target=0x80000001 \
# RUN:   --defsym=positive_target=0x80000001 \
# RUN:   --defsym=negative_target=0x7fffffff -o /dev/null %t.o 2>&1 | \
# RUN:   FileCheck %s --check-prefix=BELOW-MIN

# OBJECT:      Name: .data
# OBJECT:      Name: .debug_ia16
# OBJECT:      Flags [ (0x0)
# OBJECT:      Section {{.*}} .rel.data {
# OBJECT-NEXT:   0x0 R_386_32 first
# OBJECT-NEXT:   0x0 R_386_SUB32 second
# OBJECT-NEXT:   0x4 R_386_32 zero
# OBJECT-NEXT:   0x4 R_386_SUB32 zero_target
# OBJECT-NEXT:   0x8 R_386_32 one
# OBJECT-NEXT:   0x8 R_386_SUB32 positive_target
# OBJECT-NEXT:   0xC R_386_32 negative_one
# OBJECT-NEXT:   0xC R_386_SUB32 negative_target
# OBJECT:      Section {{.*}} .rel.debug_ia16 {
# OBJECT-NEXT:   0x0 R_386_32 first
# OBJECT-NEXT:   0x0 R_386_SUB32 second
# OBJECT-NEXT:   0x4 R_386_32 zero
# OBJECT-NEXT:   0x4 R_386_SUB32 zero_target
# OBJECT-NEXT:   0x8 R_386_32 one
# OBJECT-NEXT:   0x8 R_386_SUB32 positive_target
# OBJECT-NEXT:   0xC R_386_32 negative_one
# OBJECT-NEXT:   0xC R_386_SUB32 negative_target

# Allocated and nonallocated expressions have identical cumulative results.
# The first word is 1 - 2 = -1. The remaining words exercise the exact signed
# minimum with zero, positive, and negative cumulative addends.
# CONTENTS:      Contents of section .data:
# CONTENTS-NEXT:  ffffffff 00000080 00000080 00000080
# CONTENTS:      Contents of section .debug_ia16:
# CONTENTS-NEXT:  ffffffff 00000080 00000080 00000080

# RETAINED:      Section {{.*}} .rel.data {
# RETAINED-NEXT:   {{.*}} R_386_32 first
# RETAINED-NEXT:   {{.*}} R_386_SUB32 second
# RETAINED:      Section {{.*}} .rel.debug_ia16 {
# RETAINED-NEXT:   0x0 R_386_32 first
# RETAINED-NEXT:   0x0 R_386_SUB32 second

# BELOW-MIN: error: {{.*}}:(.debug_ia16+0x4): relocation R_386_SUB32 out of range: -2147483649 is not in [-2147483648, 2147483647]; references 'zero_target'

.macro expressions
.long 0
.reloc .-4, R_386_32, first
.reloc .-4, R_386_SUB32, second
.long 0
.reloc .-4, R_386_32, zero
.reloc .-4, R_386_SUB32, zero_target
.long 0
.reloc .-4, R_386_32, one
.reloc .-4, R_386_SUB32, positive_target
.long 0
.reloc .-4, R_386_32, negative_one
.reloc .-4, R_386_SUB32, negative_target
.endm

.section .data,"aw",@progbits
expressions

.section .debug_ia16,"",@progbits
expressions
