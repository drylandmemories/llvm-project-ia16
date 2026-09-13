// RUN: %clang_cc1 -triple ia16-unknown-none-elf -std=c11 -verify %s

_Atomic(int) value; // expected-error {{atomic types are not supported in 'ia16-unknown-none-elf'}}
_Thread_local int tls_value; // expected-error {{thread-local storage is not supported for the current target}}
