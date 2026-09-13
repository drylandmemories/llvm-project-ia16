# REQUIRES: x86

# RUN: rm -rf %t && split-file %s %t
# RUN: llvm-mc -filetype=obj -triple=i386 %t/protected.s -o %t/protected.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/protected-empty.s -o %t/protected-empty.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/real.s -o %t/real.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/real-empty.s -o %t/real-empty.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/unmarked.s -o %t/unmarked.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/bad-nul.s -o %t/bad-nul.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/bad-mode.s -o %t/bad-mode.o

## A protected-mode output may not apply real-mode paragraph semantics.
# RUN: not ld.lld -m elf_i386 --entry=0 --defsym=target=0x12345 -o /dev/null \
# RUN:   %t/protected.o 2>&1 | FileCheck %s --check-prefix=PROTECTED
# PROTECTED: error: {{.*}}protected.o:(.data+0x0): relocation R_386_SEG16 is invalid in IA-16 protected-mode output

## An unmarked handwritten relocation is also rejected when another input
## establishes that the output is protected mode.
# RUN: not ld.lld -m elf_i386 --entry=0 --defsym=target=0x12345 -o /dev/null \
# RUN:   %t/protected-empty.o %t/unmarked.o 2>&1 | \
# RUN:   FileCheck %s --check-prefix=UNMARKED-PROTECTED
# UNMARKED-PROTECTED: error: {{.*}}unmarked.o:(.data+0x0): relocation R_386_SEG16 is invalid in IA-16 protected-mode output

## Explicit real-mode input retains the accepted SEGELF behavior.
# RUN: ld.lld -m elf_i386 --entry=0 --defsym=target=0xfffff -o %t/real %t/real.o
# RUN: llvm-objdump -s -j .data %t/real | FileCheck %s --check-prefix=REAL
# RUN: llvm-readelf -n %t/real | FileCheck %s --check-prefix=REAL-NOTE
# REAL: Contents of section .data:
# REAL: ffff
# REAL-NOTE: Displaying notes found in: .note.ia16.mode
# REAL-NOTE: IA16
# REAL-NOTE: description data: 49 41 31 36 2d 4d 4f 44 45 3a 72 65 61 6c 00

## Conflicting explicit modes and malformed mode notes fail closed.
# RUN: not ld.lld -m elf_i386 --entry=0 -o /dev/null %t/protected-empty.o \
# RUN:   %t/real-empty.o 2>&1 | FileCheck %s --check-prefix=MIXED
# MIXED: error: {{.*}}real-empty.o: IA-16 real mode is incompatible with {{.*}}protected-empty.o (protected mode)
# RUN: not ld.lld -m elf_i386 --entry=0 -o /dev/null %t/bad-nul.o 2>&1 | \
# RUN:   FileCheck %s --check-prefix=BAD-NUL
# BAD-NUL: error: {{.*}}bad-nul.o:(.note.ia16.mode+0x0): descriptor is not NUL-terminated
# RUN: not ld.lld -m elf_i386 --entry=0 -o /dev/null %t/bad-mode.o 2>&1 | \
# RUN:   FileCheck %s --check-prefix=BAD-MODE
# BAD-MODE: error: {{.*}}bad-mode.o:(.note.ia16.mode+0x0): invalid IA-16 mode descriptor

#--- protected.s
.section .note.ia16.mode,"",@note
.p2align 2
.long 5
.long 20
.long 2
.asciz "IA16"
.p2align 2
.asciz "IA16-MODE:protected"
.p2align 2

.section .data,"a",@progbits
.short 0
.reloc .-2, R_386_SEG16, target

#--- protected-empty.s
.section .note.ia16.mode,"",@note
.p2align 2
.long 5
.long 20
.long 2
.asciz "IA16"
.p2align 2
.asciz "IA16-MODE:protected"
.p2align 2

#--- real.s
.section .note.ia16.mode,"",@note
.p2align 2
.long 5
.long 15
.long 2
.asciz "IA16"
.p2align 2
.asciz "IA16-MODE:real"
.p2align 2

.section .data,"a",@progbits
.short 0
.reloc .-2, R_386_SEG16, target

#--- real-empty.s
.section .note.ia16.mode,"",@note
.p2align 2
.long 5
.long 15
.long 2
.asciz "IA16"
.p2align 2
.asciz "IA16-MODE:real"
.p2align 2

#--- unmarked.s
.section .data,"a",@progbits
.short 0
.reloc .-2, R_386_SEG16, target

#--- bad-nul.s
.section .note.ia16.mode,"",@note
.p2align 2
.long 5
.long 19
.long 2
.asciz "IA16"
.p2align 2
.ascii "IA16-MODE:protected"
.p2align 2

#--- bad-mode.s
.section .note.ia16.mode,"",@note
.p2align 2
.long 5
.long 18
.long 2
.asciz "IA16"
.p2align 2
.asciz "IA16-MODE:unknown"
.p2align 2
