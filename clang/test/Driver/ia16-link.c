// REQUIRES: x86-registered-target, lld
// RUN: %clang --target=ia16-pc-dos-elf -O1 -mcmodel=tiny -nostdlib \
// RUN:   %s -o %t.com
// RUN: %python -c "import pathlib; print(pathlib.Path(r'%t.com').read_bytes().hex())" \
// RUN:   | FileCheck %s --check-prefix=COMHEX
// RUN: %clang --target=ia16-pc-dos-elf -O1 -mcmodel=small -nostdlib \
// RUN:   %s -o %t.exe
// RUN: %python -c "import pathlib; print(pathlib.Path(r'%t.exe').read_bytes().hex())" \
// RUN:   | FileCheck %s --check-prefix=MZHEX

// COMHEX: 5589e55dc3
// MZHEX: 4d5a25000100000002000000ffff0000feff0000000000001c000000000000005589e55dc3

void _start(void) {}
