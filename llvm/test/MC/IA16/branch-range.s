# RUN: not llvm-mc -triple=ia16-unknown-none-elf -mcpu=i8086 \
# RUN:   -filetype=obj %s -o /dev/null 2>&1 | FileCheck %s

# IA-16 must not silently relax this to the 80386 0f 8x rel16 form.
start:
  je distant
  .space 256
distant:
  ret

# CHECK: error: value of {{.*}} is too large for field of 1 byte
