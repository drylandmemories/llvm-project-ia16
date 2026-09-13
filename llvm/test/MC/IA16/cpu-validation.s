# RUN: not llvm-mc -triple=ia16 -mcpu=i8086 -defsym ERR_8086=1 %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERR8086
# RUN: llvm-mc -triple=ia16 -mcpu=i80186 -defsym OK_186=1 -show-encoding %s | FileCheck %s --check-prefix=OK186
# RUN: not llvm-mc -triple=ia16 -mcpu=i80286 -defsym ERR_POST286=1 %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERRPOST286
# RUN: not llvm-mc -triple=ia16 -mcpu=i386 -defsym ERR_CPU=1 %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERRCPU

.ifdef ERR_8086
pushw $7
shlw $2, %ax
pusha
.endif

# ERR8086: error: immediate push requires an 80186 or later
# ERR8086: error: multi-bit immediate shift requires an 80186 or later
# ERR8086: error: instruction is not available on the selected IA-16 CPU

.ifdef OK_186
pushw $7
shlw $2, %ax
pusha
.endif

# OK186: pushw $7 # encoding: [0x6a,0x07]
# OK186: shlw $2, %ax # encoding: [0xc1,0xe0,0x02]
# OK186: pushaw # encoding: [0x60]

.ifdef ERR_POST286
mov %eax, %ebx
bsfw %ax, %bx
.endif

# ERRPOST286: error: 32-bit and 64-bit encodings are not available on IA-16
# ERRPOST286: error: instruction is not available on the selected IA-16 CPU

.ifdef ERR_CPU
nop
.endif

# ERRCPU: error: invalid CPU for IA-16
