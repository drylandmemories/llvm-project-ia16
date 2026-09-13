// RUN: %clang --target=ia16-pc-dos-elf -march=i286 -mtune=i186 \
// RUN:   -mcmodel=compact -S -emit-llvm %s -### 2>&1 | FileCheck %s --check-prefix=DRIVER
// RUN: not %clang --target=ia16-pc-dos-elf -march=not-a-cpu \
// RUN:   -S -emit-llvm %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=BAD-CPU
// RUN: not %clang --target=ia16-pc-dos-elf -mtune=not-a-cpu \
// RUN:   -S -emit-llvm %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=BAD-TUNE
// RUN: not %clang --target=ia16-pc-dos-elf -mcmodel=kernel \
// RUN:   -S -emit-llvm %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=BAD-MODEL
// RUN: %clang --target=ia16-pc-dos-elf -march=i286 -mprotected-mode \
// RUN:   -S -emit-llvm %s -### 2>&1 | FileCheck %s --check-prefix=PROTECTED
// RUN: not %clang --target=ia16-pc-dos-elf -march=i8086 -mprotected-mode \
// RUN:   -S -emit-llvm %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=BAD-PROTECTED
// RUN: %clang --target=ia16-pc-dos-elf -O1 -mcmodel=tiny -nostdlib \
// RUN:   -Wl,-e,_start %s -o %t.com -### 2>&1 | FileCheck %s --check-prefix=LINK-COM
// RUN: %clang --target=ia16-pc-dos-elf -O1 -mcmodel=small -nostdlib \
// RUN:   %s -o %t.exe -### 2>&1 | FileCheck %s --check-prefix=LINK-MZ

// DRIVER: "-cc1"
// DRIVER-SAME: "-triple" "ia16-pc-dos-elf"
// DRIVER-DAG: "-target-cpu" "i286"
// DRIVER-DAG: "-tune-cpu" "i186"
// DRIVER-DAG: "-mcmodel=compact"

// BAD-CPU: error: unknown target CPU 'not-a-cpu'
// BAD-TUNE: error: unknown target CPU 'not-a-cpu'
// BAD-MODEL: error: unsupported argument 'kernel' to option '-mcmodel=' for target 'ia16-pc-dos-elf'
// PROTECTED: "-target-feature" "+protected-mode"
// BAD-PROTECTED: error: invalid feature combination: IA-16 protected mode requires -march=i80286 or -march=i286

// LINK-COM:      "{{.*}}ld.lld" "-m" "elf_i386" "--image-base=0" "--emit-relocs" "--gc-sections" "-N" "-Ttext=0x100" "--entry=_start"
// LINK-COM-SAME: "-e" "_start"
// LINK-COM-NEXT: "{{.*}}llvm-ia16-mz" "--format=com"

// LINK-MZ:      "{{.*}}ld.lld" "-m" "elf_i386" "--image-base=0" "--emit-relocs" "--gc-sections" "-N" "-Ttext=0" "--entry=_start"
// LINK-MZ-NEXT: "{{.*}}llvm-ia16-mz" "--format=mz"

void _start(void) {}
