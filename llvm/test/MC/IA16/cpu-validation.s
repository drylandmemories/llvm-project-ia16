# RUN: not llvm-mc -triple=ia16 -mcpu=i8086 -defsym ERR_8086=1 %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERR8086
# RUN: not llvm-mc -triple=ia16 -mcpu=i8088 -defsym ERR_8086=1 %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERR8086
# RUN: llvm-mc -triple=ia16 -mcpu=i80186 -defsym OK_186=1 -show-encoding %s | FileCheck %s --check-prefix=OK186
# RUN: llvm-mc -triple=ia16 -mcpu=i80188 -defsym OK_186=1 -show-encoding %s | FileCheck %s --check-prefix=OK186
# RUN: llvm-mc -triple=ia16 -mcpu=i80286 -defsym OK_186=1 -show-encoding %s | FileCheck %s --check-prefix=OK186
# RUN: not llvm-mc -triple=ia16 -mcpu=i80186 -defsym ERR_286=1 %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERR286
# RUN: not llvm-mc -triple=ia16 -mcpu=i80188 -defsym ERR_286=1 %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERR286
# RUN: llvm-mc -triple=ia16 -mcpu=i80286 -defsym OK_286=1 -show-encoding %s | FileCheck %s --check-prefix=OK286
# RUN: not llvm-mc -triple=ia16 -mcpu=i80286 -defsym ERR_POST286=1 %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERRPOST286
# RUN: not llvm-mc -triple=ia16 -mcpu=i386 -defsym ERR_CPU=1 %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERRCPU

.ifdef ERR_8086
pushw $7
pushw $4660
shlw $2, %ax
pusha
popa
enter $4, $0
leave
imulw $3, %ax, %bx
imulw $4660, %ax, %bx
.endif

# ERR8086: error: immediate push requires an 80186 or later
# ERR8086: error: immediate push requires an 80186 or later
# ERR8086: error: multi-bit immediate shift requires an 80186 or later
# ERR8086: error: instruction is not available on the selected IA-16 CPU
# ERR8086: error: instruction is not available on the selected IA-16 CPU
# ERR8086: error: instruction is not available on the selected IA-16 CPU
# ERR8086: error: instruction is not available on the selected IA-16 CPU
# ERR8086: error: immediate imul requires an 80186 or later
# ERR8086: error: immediate imul requires an 80186 or later

.ifdef OK_186
pushw $7
pushw $4660
shlw $2, %ax
pusha
popa
enter $4, $0
leave
imulw $3, %ax, %bx
imulw $4660, %ax, %bx
.endif

# OK186: pushw $7 # encoding: [0x6a,0x07]
# OK186: pushw $4660 # encoding: [0x68,0x34,0x12]
# OK186: shlw $2, %ax # encoding: [0xc1,0xe0,0x02]
# OK186: pushaw # encoding: [0x60]
# OK186: popaw # encoding: [0x61]
# OK186: enter $4, $0 # encoding: [0xc8,0x04,0x00,0x00]
# OK186: leave # encoding: [0xc9]
# OK186: imulw $3, %ax, %bx # encoding: [0x6b,0xd8,0x03]
# OK186: imulw $4660, %ax, %bx # encoding: [0x69,0xd8,0x34,0x12]

.ifdef ERR_286
arpl %ax, %bx
clts
lgdtw (%bx)
.endif

# ERR286: error: instruction is not available on the selected IA-16 CPU
# ERR286: error: instruction is not available on the selected IA-16 CPU
# ERR286: error: instruction is not available on the selected IA-16 CPU

.ifdef OK_286
arpl %ax, %bx
clts
larw %ax, %bx
lgdtw (%bx)
lidtw (%bx)
lldtw %ax
lmsww %ax
lslw %ax, %bx
ltrw %ax
sgdtw (%bx)
sidtw (%bx)
sldtw %ax
smsww %ax
strw %ax
verr %ax
verw %ax
.endif

# OK286: arpl %ax, %bx # encoding: [0x63,0xc3]
# OK286: clts # encoding: [0x0f,0x06]
# OK286: larw %ax, %bx # encoding: [0x0f,0x02,0xd8]
# OK286: lgdtw (%bx) # encoding: [0x0f,0x01,0x17]
# OK286: lidtw (%bx) # encoding: [0x0f,0x01,0x1f]
# OK286: lldtw %ax # encoding: [0x0f,0x00,0xd0]
# OK286: lmsww %ax # encoding: [0x0f,0x01,0xf0]
# OK286: lslw %ax, %bx # encoding: [0x0f,0x03,0xd8]
# OK286: ltrw %ax # encoding: [0x0f,0x00,0xd8]
# OK286: sgdtw (%bx) # encoding: [0x0f,0x01,0x07]
# OK286: sidtw (%bx) # encoding: [0x0f,0x01,0x0f]
# OK286: sldtw %ax # encoding: [0x0f,0x00,0xc0]
# OK286: smsww %ax # encoding: [0x0f,0x01,0xe0]
# OK286: strw %ax # encoding: [0x0f,0x00,0xc8]
# OK286: verr %ax # encoding: [0x0f,0x00,0xe0]
# OK286: verw %ax # encoding: [0x0f,0x00,0xe8]

.ifdef ERR_POST286
mov %eax, %ebx
movw (%eax), %bx
movw %fs:(%bx), %ax
movw %gs:(%bx), %ax
bsfw %ax, %bx
shldw $2, %ax, %bx
.endif

# ERRPOST286: error: 32-bit and 64-bit encodings are not available on IA-16
# ERRPOST286: error: register is not available on IA-16
# ERRPOST286: error: register is not available on IA-16
# ERRPOST286: error: register is not available on IA-16
# ERRPOST286: error: instruction is not available on the selected IA-16 CPU
# ERRPOST286: error: instruction is not available on the selected IA-16 CPU

.ifdef ERR_CPU
nop
.endif

# ERRCPU: error: invalid CPU for IA-16
