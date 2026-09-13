# REQUIRES: x86

# RUN: llvm-mc -triple=i386 -filetype=obj %s -o %t.o
# RUN: llvm-readobj -r %t.o | FileCheck %s --check-prefix=RELOCS
# RUN: ld.lld -m elf_i386 --defsym=target=0x12345 \
# RUN:   --defsym=base=0x12000 -o %t %t.o
# RUN: llvm-objdump -s -j .data %t | FileCheck %s --check-prefix=CONTENTS

# RELOCS:      R_386_SEG16 target
# RELOCS-NEXT: R_386_HUGE8 target
# RELOCS-NEXT: R_386_SUB16 target
# RELOCS-NEXT: R_386_SUB32 target

# Segment and normalized offset, subtraction with a nonzero addend, and
# cumulative symbol differences. The R_386_16 intermediate value overflows 16
# bits, but the paired result is representable.
# CONTENTS: Contents of section .data:
# CONTENTS: 341205c2 dcc2dcfe ff4e034e 030000

.section .data,"a",@progbits
.short 0
.reloc .-2, R_386_SEG16, target
.byte 0
.reloc .-1, R_386_HUGE8, target
.short 7
.reloc .-2, R_386_SUB16, target
.long 7
.reloc .-4, R_386_SUB32, target
.short 9
.reloc .-2, R_386_16, target
.reloc .-2, R_386_SUB16, base
.long 9
.reloc .-4, R_386_32, target
.reloc .-4, R_386_SUB32, base
