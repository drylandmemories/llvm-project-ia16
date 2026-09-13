# REQUIRES: x86

# RUN: llvm-mc -triple=i386 -filetype=obj %s -o %t.o
# RUN: llvm-readobj -r %t.o | FileCheck %s --check-prefix=RELOCS
# RUN: ld.lld -m elf_i386 --entry=0 --defsym=first=18 --defsym=second=3 \
# RUN:   --defsym=neg=0xffffffff --defsym=one=1 --defsym=four=4 --defsym=sixteen=16 \
# RUN:   --defsym=boundary=255 --emit-relocs %t.o -o %t
# RUN: llvm-objdump -s -j .data -j .nonalloc %t | FileCheck %s --check-prefix=BYTES
# RUN: llvm-readobj -r %t | FileCheck %s --check-prefix=RELOCS
# RUN: not ld.lld -m elf_i386 --entry=0 --defsym=first=18 --defsym=second=3 \
# RUN:   --defsym=neg=0xffffffff --defsym=one=1 --defsym=four=4 --defsym=sixteen=16 \
# RUN:   --defsym=boundary=256 %t.o -o /dev/null 2>&1 | FileCheck %s --check-prefix=OVER
# RUN: not ld.lld -m elf_i386 --entry=0 --defsym=first=18 --defsym=second=3 \
# RUN:   --defsym=neg=0xffffffff --defsym=one=1 --defsym=four=4 --defsym=sixteen=16 \
# RUN:   --defsym=boundary=0xffffff7f %t.o -o /dev/null 2>&1 | FileCheck %s --check-prefix=UNDER

# Both section paths must use the previous relocation's result as A.
# Cover initial zero/positive/negative addends, signed-byte boundaries,
# a three-relocation chain, and standalone HUGE8 at distinct offsets.
# BYTES: Contents of section .data:
# BYTES-NEXT: {{[0-9a-f]+}} 05060401 04020900 010f0f00
# BYTES: Contents of section .nonalloc:
# BYTES-NEXT: {{[0-9a-f]+}} 05060401 04020900 010f0f00

# The first relocation must still diagnose overflow before normalization.
# OVER: error: {{.*}}:(.data+0x5): relocation R_386_8 out of range: 256 is not in [-128, 255]
# UNDER: error: {{.*}}:(.data+0x5): relocation R_386_8 out of range: -129 is not in [-128, 255]

# RELOCS: .rel.data {
# RELOCS: R_386_8
# RELOCS-NEXT: {{.*}} R_386_HUGE8
# RELOCS: R_386_8 boundary
# RELOCS-NEXT: {{.*}} R_386_HUGE8
# RELOCS-NEXT: {{.*}} R_386_8
# RELOCS-NEXT: {{.*}} R_386_HUGE8
# RELOCS-NEXT: {{.*}} R_386_HUGE8
# RELOCS: .rel.nonalloc {
# RELOCS: R_386_8
# RELOCS-NEXT: {{.*}} R_386_HUGE8
# RELOCS: R_386_8 boundary
# RELOCS-NEXT: {{.*}} R_386_HUGE8
# RELOCS-NEXT: {{.*}} R_386_8
# RELOCS-NEXT: {{.*}} R_386_HUGE8
# RELOCS-NEXT: {{.*}} R_386_HUGE8

.macro pair initial, first, second
.byte \initial
.reloc .-1, R_386_8, \first
.reloc .-1, R_386_HUGE8, \second
.endm

.macro cases
pair 0, first, second
pair 1, first, second
pair -1, first, second
pair 127, neg, second
pair -128, one, second
pair 0, boundary, second
pair 0, first, second
.reloc .-1, R_386_HUGE8, four
.byte 0, 1, -1, 127, -128
.reloc .-5, R_386_HUGE8, sixteen
.reloc .-4, R_386_HUGE8, sixteen
.reloc .-3, R_386_HUGE8, sixteen
.reloc .-2, R_386_HUGE8, sixteen
.reloc .-1, R_386_HUGE8, sixteen
.endm

.section .data,"aw",@progbits
cases
.section .nonalloc,"",@progbits
cases
