// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O0 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o0.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o2.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -Os \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.os.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -emit-obj -o %t.o %s
// RUN: llvm-readobj --file-headers --relocations %t.o | FileCheck %s \
// RUN:   --check-prefix=RELOC

int add(int a, int b) { return a + b; }
extern int callee(int, int);
int caller(void) { return callee(2, 3); }
int branch_call(int a, int b) {
  if (a > b)
    return callee(a, 1);
  return callee(b, 2);
}
unsigned char byte_inc(unsigned char value) { return value + 1; }
int global_word;
int load_global(void) { return global_word; }
void store_global(int value) { global_word = value; }
int load_pointer(const int *pointer) { return *pointer; }
void store_pointer(int *pointer, int value) { *pointer = value; }
unsigned int load_index(const unsigned int *pointer, unsigned int index) {
  return pointer[index];
}
unsigned int shift_left(unsigned int value, unsigned int count) {
  return value << count;
}
unsigned int shift_right(unsigned int value, unsigned int count) {
  return value >> count;
}
int compare_signed(int left, int right) { return left < right; }
int compare_equal(int left, int right) { return left == right; }
int compare_unsigned(unsigned int left, unsigned int right) {
  return left >= right;
}
int compare_add(int left, int right) { return (left < right) + 7; }
int call_indirect(int (*function)(int), int value) { return function(value); }
unsigned int count_leading_long(unsigned long value) {
  return __builtin_clzl(value);
}

// CHECK-LABEL: add:
// CHECK:       pushw %bp
// CHECK-NEXT:  movw %sp, %bp
// CHECK-DAG:   movw 6(%bp),
// CHECK-DAG:   movw 4(%bp),
// CHECK:       addw
// CHECK:       popw %bp
// CHECK-NEXT:  retw

// CHECK-LABEL: caller:
// CHECK:       movw $3, %ax
// CHECK-NEXT:  pushw %ax
// CHECK-NEXT:  movw $2, %ax
// CHECK-NEXT:  pushw %ax
// CHECK-NEXT:  callw callee
// CHECK:       addw $4,
// CHECK:       retw

// CHECK-LABEL: branch_call:
// CHECK:       cmpw
// CHECK:       jle
// CHECK:       callw callee
// CHECK:       callw callee
// CHECK:       retw

// CHECK-LABEL: byte_inc:
// CHECK:       movb 4(%bp),
// CHECK:       addb
// CHECK:       xorw
// CHECK:       retw

// CHECK-LABEL: load_global:
// CHECK:       movw global_word,
// CHECK:       retw

// CHECK-LABEL: store_global:
// CHECK:       movw {{.*}}, global_word
// CHECK:       retw

// CHECK-LABEL: load_pointer:
// CHECK:       movw %bx, -2(%bp)
// CHECK:       movw ({{%([bs]x|bp|[sd]i)}}),
// CHECK:       movw -2(%bp), %bx
// CHECK:       retw

// RELOC:      Format: elf32-i386
// RELOC:      Machine: EM_386
// RELOC-COUNT-3: R_386_PC16
// RELOC-COUNT-2: R_386_16 global_word

// CHECK-LABEL: store_pointer:
// CHECK:       movw %bx, -2(%bp)
// CHECK:       movw {{.*}}, ({{%([bs]x|bp|[sd]i)}})
// CHECK:       movw -2(%bp), %bx
// CHECK:       retw

// CHECK-LABEL: load_index:
// CHECK:       shlw
// CHECK:       addw
// CHECK:       movw ({{%([bs]x|bp|[sd]i)}}),
// CHECK:       retw

// CHECK-LABEL: shift_left:
// CHECK:       movb {{.*}}, %cl
// CHECK:       shlw %cl,
// CHECK:       retw

// CHECK-LABEL: shift_right:
// CHECK:       movb {{.*}}, %cl
// CHECK:       shrw %cl,
// CHECK:       retw

// CHECK-LABEL: compare_signed:
// CHECK:       movw $1,
// CHECK:       cmpw
// CHECK:       jl
// CHECK:       movw $0,
// CHECK:       retw

// CHECK-LABEL: compare_equal:
// CHECK:       cmpw
// CHECK:       je
// CHECK:       retw

// CHECK-LABEL: compare_unsigned:
// CHECK:       cmpw
// CHECK:       jae
// CHECK:       retw

// CHECK-LABEL: compare_add:
// CHECK:       cmpw
// CHECK:       jl
// CHECK:       retw

// CHECK-LABEL: call_indirect:
// CHECK:       pushw
// CHECK:       callw *%{{(ax|bx|cx|dx|si|di|bp)}}
// CHECK:       addw $2,
// CHECK:       retw

// SelectionDAG range assertions carry no run-time operation and must not block
// selection of the narrowed leading-zero result.
// CHECK-LABEL: count_leading_long:
// CHECK:       retw
