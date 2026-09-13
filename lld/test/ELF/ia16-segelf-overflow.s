# REQUIRES: x86

# RUN: llvm-mc -triple=i386 -filetype=obj %s -o %t.o
# RUN: llvm-readobj -r %t.o | FileCheck %s --check-prefix=RELOCS
# RUN: ld.lld -m elf_i386 --defsym=target=0xfffff -o %t %t.o
# RUN: llvm-objdump -s -j .data %t | FileCheck %s --check-prefix=CONTENTS
# RUN: not ld.lld -m elf_i386 --defsym=target=0x100000 -o /dev/null %t.o 2>&1 \
# RUN:   | FileCheck %s --check-prefix=OVERFLOW

# RELOCS: R_386_SEG16 target

# CONTENTS: Contents of section .data:
# CONTENTS: ffff

# OVERFLOW: error: {{.*}}:(.data+0x0): relocation R_386_SEG16 out of range: 65536 is not in [-32768, 65535]; references 'target'

.section .data,"a",@progbits
.short 0
.reloc .-2, R_386_SEG16, target
