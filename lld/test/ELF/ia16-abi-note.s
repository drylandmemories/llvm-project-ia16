# REQUIRES: x86

# RUN: rm -rf %t && split-file %s %t
# RUN: llvm-mc -filetype=obj -triple=i386 %t/v02.s -o %t/v02.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/v02.s -o %t/v02-copy.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/v01.s -o %t/v01.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/missing.s -o %t/missing.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/bad-nul.s -o %t/bad-nul.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/conflict.s -o %t/conflict.o

## Equal recorded versions link, and their notes are retained in the output.
# RUN: ld.lld --entry=0 %t/v02.o %t/v02-copy.o -o %t/same
# RUN: llvm-readelf -n %t/same | FileCheck %s --check-prefix=NOTE
# NOTE: Displaying notes found in: .note.ia16.abi
# NOTE: IA16
# NOTE-NEXT: description data: 49 41 31 36 2d 41 42 49 3a 30 2e 32 00
# NOTE: IA16
# NOTE-NEXT: description data: 49 41 31 36 2d 41 42 49 3a 30 2e 32 00

## Handwritten assembly without a version note remains compatible.
# RUN: ld.lld --entry=0 %t/v02.o %t/missing.o -o %t/with-missing

## Different recorded ABI versions are incompatible.
# RUN: not ld.lld --entry=0 %t/v02.o %t/v01.o -o %t/mismatch 2>&1 | \
# RUN:   FileCheck %s --check-prefix=MISMATCH
# MISMATCH: error: {{.*}}v01.o: IA-16 ABI IA16-ABI:0.1 is incompatible with {{.*}}v02.o (IA16-ABI:0.2)

## Malformed and internally conflicting IA-16 notes are rejected at input.
# RUN: not ld.lld --entry=0 %t/bad-nul.o -o %t/bad 2>&1 | \
# RUN:   FileCheck %s --check-prefix=BAD-NUL
# BAD-NUL: error: {{.*}}bad-nul.o:(.note.ia16.abi+0x0): descriptor is not NUL-terminated
# RUN: not ld.lld --entry=0 %t/conflict.o -o %t/conflict 2>&1 | \
# RUN:   FileCheck %s --check-prefix=CONFLICT
# CONFLICT: error: {{.*}}conflict.o:(.note.ia16.abi+0x24): conflicting IA-16 ABI descriptors IA16-ABI:0.2 and IA16-ABI:0.1

#--- v02.s
.section .note.ia16.abi,"",@note
.p2align 2
.long 5
.long 13
.long 1
.asciz "IA16"
.p2align 2
.asciz "IA16-ABI:0.2"
.p2align 2

#--- v01.s
.section .note.ia16.abi,"",@note
.p2align 2
.long 5
.long 13
.long 1
.asciz "IA16"
.p2align 2
.asciz "IA16-ABI:0.1"
.p2align 2

#--- missing.s
.text
.byte 0x90

#--- bad-nul.s
.section .note.ia16.abi,"",@note
.p2align 2
.long 5
.long 12
.long 1
.asciz "IA16"
.p2align 2
.ascii "IA16-ABI:0.2"
.p2align 2

#--- conflict.s
.section .note.ia16.abi,"",@note
.p2align 2
.long 5
.long 13
.long 1
.asciz "IA16"
.p2align 2
.asciz "IA16-ABI:0.2"
.p2align 2
.long 5
.long 13
.long 1
.asciz "IA16"
.p2align 2
.asciz "IA16-ABI:0.1"
.p2align 2
