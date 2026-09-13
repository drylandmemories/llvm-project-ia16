// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O0 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s \
// RUN:   --check-prefix=O0
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s \
// RUN:   --check-prefix=OPT
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o2.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -Os \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.os.o %s

int dense_switch(int value) {
  switch (value) {
  case 0:
    return 11;
  case 1:
    return 13;
  case 2:
    return 17;
  case 3:
    return 19;
  case 4:
    return 23;
  case 5:
    return 29;
  default:
    return 31;
  }
}

// Until the dedicated segmented switch-table lowering is implemented, a
// control-flow switch expands to 8086 comparisons and short/near branches.
// O0-LABEL: dense_switch:
// O0:       cmpw
// O0:       j{{e|ne}}
// O0-NOT:   LJTI
// O0:       retw

// Optimized constant-result switches may use an ordinary DS-relative lookup
// table. Its near address is materialized as a 16-bit immediate.
// OPT-LABEL: dense_switch:
// OPT:       movw $.Lswitch.table.dense_switch,
// OPT:       movw ({{%[a-z]+}}),
// OPT-NOT:   LJTI
// OPT:       retw
