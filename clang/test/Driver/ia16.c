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
