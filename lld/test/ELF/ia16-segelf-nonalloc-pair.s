# REQUIRES: x86

# RUN: llvm-mc -triple=i386 -filetype=obj %s -o %t.o
# RUN: llvm-readobj --section-headers --relocations %t.o | \
# RUN:   FileCheck %s --check-prefix=OBJECT
# RUN: ld.lld -m elf_i386 --entry=0 --defsym=target=0x1234 \
# RUN:   --defsym=base=0x1200 -o %t %t.o
# RUN: llvm-objdump -s -j .debug_ia16 %t | \
# RUN:   FileCheck %s --check-prefix=CONTENTS

# OBJECT:      Name: .debug_ia16
# OBJECT:      Flags [ (0x0)
# OBJECT:      Section {{.*}} .rel.debug_ia16 {
# OBJECT-NEXT:   0x0 R_386_16 target
# OBJECT-NEXT:   0x0 R_386_SUB16 base

# Same-offset REL relocations are cumulative even in a non-SHF_ALLOC section:
# target + 9 - base = 0x003d.
# CONTENTS: Contents of section .debug_ia16:
# CONTENTS: 0000 3d00

.section .debug_ia16,"",@progbits
.short 9
.reloc .-2, R_386_16, target
.reloc .-2, R_386_SUB16, base
