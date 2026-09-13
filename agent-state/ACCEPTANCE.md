# LLVM IA-16 acceptance register

Why: Passing a narrow compiler slice can expose real progress while still
leaving object-format, parser, ABI, integration, or runtime requirements
unproved. This register prevents evidence from being promoted beyond its layer.

Scope: Record the authoritative proof required for the complete Gates 1–5
roadmap and the strongest currently retained evidence. `pending` means required
proof is absent or incomplete; it does not mean the implementation is absent.

## Evidence rules

- Bind every result to a full source commit, exact command/configuration, and
  retained output. Keep source, build, test, integration, executable scan,
  emulator runtime, and packaged-toolchain evidence separate.
- A test count proves only the selected tests. Historical evidence that cannot
  be reproduced from retained commands and artifacts is context, not release
  acceptance.
- No gate closes from commits or filenames alone. All requirements assigned to
  the gate must pass from one exact candidate; Gate 5 release requires the
  complete suites from the same pinned commit.

## Historical baseline requiring reproduction

| Evidence | Current disposition | Limitation / next proof |
| --- | --- | --- |
| Source history through `8d2a0038806ae65ad690c82776df77ab214c4d94` | retained source evidence | Reproduce build/tests after task-manager integration; commits do not prove gate closure. |
| Historical IA-16 lit slice: 25 selected tests passed | historical test evidence only | The selection omitted SEGELF relocation coverage and IA-16 parser unit tests; it cannot prove Gate 1. Recover the exact selection, add omissions, and rerun from one full SHA. |
| Historical `clang` and IA-16 compiler-rt builds with two workers | historical build evidence only | Re-run using documented environment and exact SHA; archive symbol/closure checks remain separately required. |
| Historical division `.COM` probe on explicit 8086, 80186, and 80286 | retained fixture and historical runtime evidence | Persisted fixture must be rebuilt from a pinned source, opcode-scanned, and re-executed with emulator configs/logs retained. |
| Historical broad LLD run with five non-IA16 failures | incomplete integration evidence | Reproduce and classify against the current pinned source; no `check-lld` clean claim exists. |

## Gate register

| ID | Required proof | Status |
| --- | --- | --- |
| G1-ABI | Versioned ABI spec covers layouts, frames, preserved registers, returns, variadics, all pointer/call forms, segment assumptions, interrupts, and all six models. | pending audit |
| G1-TRIPLE | Parser unit tests and driver/lit tests prove both triples, DOS classification, CPU/tune aliases, model/mode options, data layouts, macros, and diagnostics. | pending; parser unit tests explicitly missing from historical 25-test slice |
| G1-MC | Per-CPU positive and rejection tests plus executable opcode scans prove genuine 8086/8088/80186/80188/80286 generation and reject all post-286 resources. | pending comprehensive matrix |
| G1-SEGELF | MC, ELF writer/parser, and LLD tests prove `R_386_SEG16`, `R_386_SUB16`, `R_386_SUB32`, `R_386_HUGE8`, malformed cases, overflow, retention, and semantics. | pending; SEGELF tests explicitly missing from historical 25-test slice |
| G1-DIFF | Frozen gcc-ia16 `20240218` fixtures establish compatible cdecl/source behavior without copied implementation. | pending |
| G2-CODEGEN | SelectionDAG legality, register/address constraints, lowering categories, branch relaxation, compiler-rt closure, and tiny/small freestanding programs pass for 8086. | partial historical evidence; full matrix pending |
| G3-CLANG | TargetInfo, data model, qualifiers/address spaces, builtins, attributes, ABI lowering, diagnostics, and optimizer-safety tests pass. | pending complete interface audit |
| G3-ELF-DOS | Segmented ELF link, model scripts, one-command driver flow, COM/MZ headers/relocations, overflow diagnostics, and hosted tiny/small runtime pass. | partial historical source; acceptance pending |
| G4-MODELS | Medium >64 KiB code, compact >64 KiB data, and large multi-segment code/data pass cross-segment runtime tests. | pending |
| G4-PM286 | GDT entry, selectors, far calls/data, interrupts, deterministic signature, and protected-huge diagnostic pass in 80286 configuration. | pending |
| G5-HUGE | Normalized huge objects and pointer walks cross two 64 KiB boundaries in real mode with relocation and optimizer coverage. | pending |
| G5-RUNTIME | Compiler-rt multiword/software-float builtins and six baseline-8086 Newlib multilibs pass symbol, ABI, and runtime suites. | partial source; complete runtime/sysroot pending |
| G5-RANDOM | Random arithmetic/control-flow/stack/aggregate/pointer tests match host at `-O0`, `-O2`, and `-Os`. | pending |
| G5-DIFF | Opposite-compiler caller/callee cross-links pass scalars, aggregates, variadics, far pointers, and calling conventions. | pending |
| G5-EMULATOR | Opcode-approved COM/MZ/model programs pass separate 8086, 80186, and 80286 DOSBox-X configurations. | pending complete matrix |
| RELEASE | `check-llvm`, `check-clang`, `check-lld`, differential, opcode, and emulator suites are all clean from one pinned commit, then a reproducible macOS package is verified. | not met |

## Deferred-scope register

OMF, legacy object compatibility, C++, exceptions, RTTI, dynamic linking, TLS,
sanitizers, JIT, 8087/80287 acceptance, protected-mode huge pointers, and
upstream target admission are excluded from the initial release. A passing
initial release must not be described as implementing these items.

## Next acceptance outcome

Run `IA-16 — Reproducible baseline acceptance` first. It may add or repair only
repository-owned acceptance harnesses and tests; it must not repair compiler,
linker, runtime, or packer implementation. It must reconstruct the current
build and exact IA-16 selection at two workers; add SEGELF relocation and
target-parser unit-test coverage; and exercise arithmetic, signed/unsigned
division edge cases, and 16-bit byte swaps at `-O0`, `-O2`, and `-Os`.

Every run uses deterministic fresh directories and bounded emulator execution,
records full source/tool/CPU identities, accounts for opcode coverage rather
than merely reporting that a scan ran, maps each requirement to retained
evidence, and identifies the next bounded gap. Persist commands/results and
return pass/partial/blocked without broadening a discovered implementation
defect; such a defect becomes a separate later Feature or Bug.
