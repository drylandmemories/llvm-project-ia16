# LLVM IA-16 Gates 1–5 roadmap

Why: LLVM's existing `-m16` keeps the i386 frontend and ABI, so it cannot by
itself compile genuine 16-bit C for the 8086 through 80286. This project must
provide a distinct architecture, segmented ABI, pre-386 code generator, object
and executable formats, runtimes, and acceptance evidence.

Scope: This is the complete production-oriented LLVM 22.1.7 IA-16 plan. It is
normative scope, not a claim that a gate is complete. Evidence and dispositions
are tracked separately in `agent-state/ACCEPTANCE.md`.

## Fixed target and ABI contract

- Build from `llvmorg-22.1.7` in the fully local
  `/Users/tedbullock/Developer/llvm-project-ia16` checkout. Maintain a distinct
  `Triple::ia16` target registered within the X86 component without changing
  i386 `-m16` behavior.
- Accept canonical triples `ia16-unknown-none-elf` and `ia16-pc-dos-elf`, add
  DOS OS classification, and default to `i8086`.
- Accept `-march=i8086|i8088|i80186|i80188|i80286`, aliases `i186` and `i286`,
  and matching `-mtune` values. Instruction availability and tuning remain
  separate.
- Accept `-mcmodel=tiny|small|medium|compact|large|huge`,
  `-mprotected-mode`, and `-mreal-mode`, with gcc-ia16-compatible target
  macros.
- Define 8-bit `char`; 16-bit `short` and `int`; 32-bit `long`; 64-bit `long
  long`; IEEE binary32/binary64 software floating point; and 16-bit `size_t`
  and `ptrdiff_t`, except that the huge model uses 32-bit forms.
- Support GNU-compatible `__near`, `__far`, `__huge`, and `__seg_ss`; `cdecl`,
  `stdcall`, and `regparmcall`; near/far-section, interrupt, and DS-preservation
  attributes. gcc-ia16 documented behavior is the compatibility baseline;
  GPL implementation code must not be copied.

| Model | Calls and function pointers | Default data pointers | Required layout |
| --- | --- | --- | --- |
| Tiny | near, 16-bit | near, 16-bit | `CS=DS=SS`; one 64 KiB space |
| Small | near, 16-bit | near, 16-bit | one code and one data/stack segment |
| Medium | far, 32-bit | near, 16-bit | multiple code segments |
| Compact | near, 16-bit | far, 32-bit | multiple data segments |
| Large | far, 32-bit | far, 32-bit | multiple code and data segments |
| Huge | far, 32-bit | normalized huge, 32-bit | objects and arithmetic may cross real-mode segment boundaries |

- Preserve the frozen ABI address-space numbers: AS0 near/DS data, AS1 far
  data, AS2 normalized huge data, AS3 stack/SS data, AS4 near code, and AS5 far
  code. They are IR-linking interfaces and must not be renumbered. Far pointers
  have 32-bit representations with 16-bit GEP indexes; huge pointers have
  32-bit representations and indexes.
- Far arithmetic must not cross its 64 KiB offset range. Huge pointers are
  normalized and may cross real-mode segments. Protected-mode far values are
  selector:offset pairs; diagnose cross-selector huge arithmetic as unsupported.
- Produce segmented ELF32 objects with `EM_386` interoperability and DOS `.COM`
  or MZ executables. Static linking and software floating point are the initial
  production scope.

## Gate 1 — ABI, identity, MC validation, and relocations

1. Publish a versioned IA-16 ABI specification covering every model: object
   layout, stack frames, register preservation, scalar and aggregate returns,
   variadics, near/far calls and pointers, conversions, segment assumptions,
   interrupt frames, and protected-mode constraints.
2. Match gcc-ia16 cdecl ABI and source behavior where documented. Freeze
   differential fixtures against gcc-ia16 release `20240218` without copying
   its GPL implementation.
3. Implement and test target/OS triple parsing, normalization, aliases, CPU and
   tune selection, model/mode options, data layouts, ABI version notes, and
   diagnostics.
4. Audit reusable MC records and attach minimum-generation predicates. Reject
   386 registers, operand/address-size overrides, FS/GS, and every post-286
   opcode. For 8086, expand immediate multi-bit shifts, immediate pushes,
   immediate IMUL, ENTER/LEAVE, PUSHA/POPA, and all later instructions. Enable
   only the appropriate 80186 additions and required 80286 application or
   protected-mode instructions.
5. Freeze and implement the SEGELF relocation family and exact semantics:
   `R_386_SEG16`, `R_386_SUB16`, `R_386_SUB32`, and `R_386_HUGE8`, alongside
   existing `R_386_16` and `R_386_PC16`. Test assembler, ELF writer, parser,
   linker application, overflow, malformed input, and relocation retention.

## Gate 2 — freestanding genuine-8086 tiny/small code generation

1. Give IA-16 separate TargetMachine, subtarget, SelectionDAG lowering and
   selection, register information, frame lowering, calling conventions,
   branch relaxation, and machine-function state while reusing only valid X86
   TableGen, MC encoder, assembler, disassembler, and ELF writer foundations.
2. Use SelectionDAG initially. Disable FastISel and GlobalISel until the full
   conformance suite passes.
3. Make `i8` and `i16` legal. Expand `i32` and `i64` into register pairs or
   compiler-runtime calls. Default all floating point to software.
4. Restrict allocation to AX/BX/CX/DX/SI/DI/BP with SP reserved. Use
   AL/BL/CL/DL and AH/BH/CH/DH for bytes. Encode only genuine
   BX/BP/SI/DI-based 16-bit address combinations.
5. Custom-lower fixed-register multiply/divide, variable shifts through CL,
   32/64-bit arithmetic, segment-register operations, near calls/returns,
   switch tables, dynamic stack allocation, structure copies, and tiny/small
   data access.
6. Add IA-16 branch relaxation that changes an out-of-range conditional branch
   into an inverted short branch plus a near jump; never emit 386 near-Jcc.
7. Pass freestanding tiny and small compiler, linker, opcode-allowlist, and
   emulator tests on explicit 8086 configuration before advancing the gate.

## Gate 3 — Clang C, explicit segmentation, and DOS output

1. Implement IA-16 TargetInfo, predefined macros, inline-assembly constraints,
   address-space qualifiers, far/huge function types, and builtins for segment
   and offset extraction and near/far construction.
2. Teach Clang ABI lowering 16-bit stack slots, DX:AX multiword values, far
   pointers, near/far calls and returns, variadics, aggregate passing/return,
   calling-convention attributes, and interrupt functions.
3. Custom-lower far loads/stores, segment-register operations, far calls and
   returns, conversions, and structure copies. Preserve required DS/SS and
   selector assumptions.
4. Prevent optimizer transformations that are unsafe across near/far/huge
   address spaces, including address-space-cast folding, huge-GEP
   reassociation, and assumptions that differently encoded far values
   necessarily alias.
5. Mark `_Atomic` unsupported and define `__STDC_NO_ATOMICS__` for the first
   complete release. Keep TLS, sanitizers, JIT, and C++ ABI support excluded.
6. Extend X86 ELF MC and LLD for the frozen segmented and normalization
   relocations. Resolve section layout to linked IA-16 ELF while retaining MZ
   relocation metadata.
7. Provide LLVM-native `llvm-ia16-mz`: tiny emits relocation-free `.COM` at PSP
   offset `0x100` and rejects overflow; other models emit MZ headers,
   paragraph-aligned segments, entry `CS:IP`, initial `SS:SP`, and loader
   relocation entries.
8. Add model-aware linker scripts and Clang driver orchestration so one command
   compiles and links a program. Pass hosted tiny/small `.COM` and MZ acceptance.

## Gate 4 — medium/compact/large and 80286 protected mode

1. Complete medium-model far code and calls beyond 64 KiB, compact-model far
   data beyond 64 KiB, and large-model simultaneous multi-segment code/data.
2. Complete selector-safe protected-mode near/far code and data for 80286,
   including valid selector dereferences, far calls/returns, and interrupt
   entry/exit. Cross-selector huge arithmetic remains unsupported and must
   produce a deliberate diagnostic.
3. Supply a separate freestanding 286 runtime that installs a GDT, enters
   protected mode, loads code/data/stack selectors, exercises near/far calls
   and pointer access, writes a deterministic success signature, and halts.
4. Pass the medium, compact, large, and protected-mode end-to-end acceptance
   described below, including opcode scanning and explicit 80286 emulation.

## Gate 5 — huge model, runtime/sysroot, hardening, and packaging

1. Implement normalized real-mode huge pointers, huge GEP/index arithmetic,
   objects spanning segment boundaries, comparisons, conversions, and
   normalization relocations. Pass a pointer walk across at least two 64 KiB
   boundaries. Do not implement protected-mode cross-selector huge arithmetic.
2. Port compiler-rt builtins for multiword arithmetic, division, shifts,
   comparisons, conversions, and IEEE binary32/binary64 software floating point.
3. Build a permissively licensed Newlib-based C sysroot in six real-mode
   multilib variants, initially using baseline 8086 code so every supported CPU
   can run the libraries.
4. Supply model-aware CRT startup/termination, `setjmp`/`longjmp`, memory/string
   routines, DOS syscall veneers, command-line/environment setup, heap
   initialization, and stack checking.
5. Harden optimization with randomized programs and cross-toolchain ABI
   differentials; package a reproducible macOS toolchain only after the release
   gate is clean from one pinned commit.

## Required acceptance and release gate

- Add lit/FileCheck coverage for triples, data layouts, Clang layouts/macros and
  diagnostics, calling conventions, every legalization category, MC encodings,
  CPU-generation rejection, all relocations, linker layouts, and MZ headers.
- Differentially test compatible C/ABI cases against gcc-ia16 in both caller
  and callee directions, cross-linking scalars, structures, variadics, far
  pointers, and calling-convention attributes.
- Generate randomized arithmetic, control-flow, stack, aggregate, and pointer
  programs; compare emulator results with a host reference at `-O0`, `-O2`,
  and `-Os`.
- Run `.COM` and MZ executables under DOSBox-X configured separately as 8086,
  80186, and 80286. Scan every executable against a per-CPU opcode allowlist
  before execution.
- End-to-end model tests must include hosted tiny and small programs; medium
  code over 64 KiB with cross-segment calls; compact aggregate data over 64 KiB;
  large multi-segment code and data; and a huge object/pointer walk crossing at
  least two 64 KiB boundaries.
- Boot and pass the 286 protected-mode harness with selector dereferences, far
  calls/returns, interrupt entry/exit, and the required huge-arithmetic
  diagnostic.
- Release only when `check-llvm`, `check-clang`, `check-lld`, the IA-16
  differential suite, and the emulator acceptance suite are clean from the
  same pinned commit.

## Serial dependency order

Advance with regressions first and in this dependency order: close the target,
ABI, parser, MC, and SEGELF foundation; prove near-code/data correctness; add
segmented Clang C and far-pointer correctness; prove hosted DOS COM/MZ; then
prove multiple segments and 80286 protected mode; finally complete huge
pointers, runtime/sysroot, optimization hardening, release suites, and package.
Do not skip an unproved dependency merely because a later feature has source
code or a narrow passing test.

## Initial-release exclusions and later work

The initial release is C-first, static-linking, software-floating-point,
segmented-ELF, and LLVM-native COM/MZ production. Defer OMF, legacy compiler
object compatibility, C++, exceptions, RTTI, dynamic linking, TLS, sanitizers,
JIT, 8087/80287 acceptance, and protected-mode huge pointers. Upstreaming is a
later project and requires an experimental-target maintainer, active community,
broad tests, and ongoing buildbot support. Do not merge the LLVM release branch
unless the complete regression suite passes afterward.

Planning estimate remains approximately 35–60 engineer-months for the credible
all-six-model toolchain and 10–15 engineer-months for a proof-quality 8086
tiny/small toolchain. The estimate does not reduce any acceptance requirement.
