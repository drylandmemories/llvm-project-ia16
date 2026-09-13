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

## Baseline evidence

| Evidence | Current disposition | Limitation / next proof |
| --- | --- | --- |
| Source through `6826ec878258c71da4c7355f6bb8544ac56476c1` | retained source evidence | The baseline and bounded MC matrix were reproduced; commits and selected tests do not prove gate closure. |
| Reproducible IA-16 selection at `5d330b7d33eb3fa73035596b82f3c21c1ec843bd` | retained selected-test evidence | 7 LLVM, 18 Clang, and 2 LLD lit tests plus the exact 7 TargetParser tests passed. This closes the historical SEGELF/parser omission only; it is not full Gate 1 or broad integration evidence. |
| Historical `clang` and IA-16 compiler-rt builds with two workers | historical build evidence only | Re-run using documented environment and exact SHA; archive symbol/closure checks remain separately required. |
| Reproducible arithmetic/division/byte-swap `.COM` probes | retained opcode-scan and emulator-runtime evidence | 9 fresh probes passed for 8086, 80186, and 80286 at `-O0`, `-O2`, and `-Os`; 3,395 instructions and all load regions were accounted. Broader model/runtime acceptance remains pending. |
| Historical broad LLD run with five non-IA16 failures | incomplete integration evidence | Reproduce and classify against the current pinned source; no `check-lld` clean claim exists. |

## Gate register

| ID | Required proof | Status |
| --- | --- | --- |
| G1-ABI | Versioned ABI spec covers layouts, frames, preserved registers, returns, variadics, all pointer/call forms, segment assumptions, interrupts, and all six models. | pending audit |
| G1-TRIPLE | Parser unit tests and driver/lit tests prove both triples, DOS classification, CPU/tune aliases, model/mode options, data layouts, macros, and diagnostics. | partial selected parser/driver baseline; full option/diagnostic matrix pending |
| G1-MC | Per-CPU positive and rejection tests plus executable opcode scans prove genuine 8086/8088/80186/80188/80286 generation and reject all post-286 resources. | bounded matrix passed at `6826ec878258c71da4c7355f6bb8544ac56476c1`: 4 selected tests, 73 instructions, 157 executable bytes, 10 load regions, 50 rejection cases, and injected `64`-`67` scanner checks; broad integration/runtime not claimed |
| G1-SEGELF | MC, ELF writer/parser, and LLD tests prove `R_386_SEG16`, `R_386_SUB16`, `R_386_SUB32`, `R_386_HUGE8`, malformed cases, overflow, retention, and semantics. | partial at `13993a92f94ea8c0224f737ebcc9458aa0ed565a`: prior protected/real-mode, SEG16, allocated-pair, and standalone SUB16 evidence remains accepted; the repaired non-SHF_ALLOC adjacent `R_386_16`/`R_386_SUB16` expression passed a focused 1/1 test, the accepted five-test slice passed 5/5, a retained ELF32 object proves flags and relocation order, linked bytes are ABI-required `0x003d`, and exact-candidate nonauthor review found no issue; standalone `R_386_SUB32` signed overflow is independently accepted at `b97d22e1214266e0f9164e34bb28ab05b4880cc1`: 7/7 retained LLD regressions, ELF32 symbol recovery, -2147483648 accepted as `0x80000000`, -2147483649 rejected with zero/negative addends, original failing fixture rejected, and relocation retention verified; cumulative/paired SUB32 policy, `R_386_HUGE8`, malformed cases, additional overflow, and broader retention remain pending |
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
| G5-EMULATOR | Opcode-approved COM/MZ/model programs pass separate 8086, 80186, and 80286 DOSBox-X configurations. | partial reproducible tiny-COM probe evidence; complete model matrix pending |
| RELEASE | `check-llvm`, `check-clang`, `check-lld`, differential, opcode, and emulator suites are all clean from one pinned commit, then a reproducible macOS package is verified. | not met |

## Deferred-scope register

OMF, legacy object compatibility, C++, exceptions, RTTI, dynamic linking, TLS,
sanitizers, JIT, 8087/80287 acceptance, protected-mode huge pointers, and
upstream target admission are excluded from the initial release. A passing
initial release must not be described as implementing these items.

## Next acceptance outcome

Run `IA-16 — Remaining Gate 1 SEGELF acceptance after SUB32 repair` as one
bounded acceptance-only task from the clean reviewed state containing product
candidate `b97d22e1214266e0f9164e34bb28ab05b4880cc1` and the reconciled register.
First retain the seven accepted IA-16 LLD regressions: ABI note,
protected/real-mode SEGELF, SEG16 overflow, allocated SEGELF relocations,
standalone SUB16 boundaries, the non-allocated pair, and standalone SUB32
boundaries. Retained SUB32 commands, tool hashes, objects, linked bytes,
retention records, and review are in `repair-standalone-sub32-overflow-v1/evidence`
under `/Users/tedbullock/Developer/llvm-ia16-tasks`.
Then exercise remaining cumulative/paired `R_386_SUB32`, `R_386_HUGE8`, malformed
relocation, additional overflow, and relocation-retention cases one at a time.
Preserve exact source/build/tool identities and separate selected-test,
object-relocation, bounded-linker, and review evidence. Stop acceptance at the
first implementation defect and checkpoint it for a separate bounded repair
when the capsule-attached user authority covers that continuation. This
register supplies factual routing, not authority. Do not repair implementation
in an acceptance-only task, run broad integration/runtime/release work, or
claim Gate 1 completion unless every registered Gate 1 requirement is
independently proved from one exact candidate.

Calibrate the successor's starting reasoning effort to its capsule and adjust
only when observed complexity warrants it, recording why. Keep every DOS-mounted
fixture name within uppercase 8.3; historical filename auditing remains later.
