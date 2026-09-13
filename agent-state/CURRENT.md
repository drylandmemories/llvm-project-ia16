# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 23
Verified-At: 2026-09-13T21:33:39Z
Verified-Against: 2f1dc83b8e0e38841f2ea291dff236a3f9265e36
Evidence-Layer: reviewed-source-build-selected-test-object-linker-repair

Owner: nonallocated cumulative SUB32 repair complete; remaining SEGELF acceptance next
Task-System-Status: adopted-artifex-2.0.0
Package-Digest: aec810ecb8a1aa788aa3c0b9de7b164fb73a9eada71de6898284fd0e8c79f3cb
Gate-1: incomplete; SEGELF repair accepted, remaining foundation acceptance pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: malformed relocations, remaining SEGELF matrix and other Gate 1 requirements lack complete acceptance evidence

Standalone SUB32 remains accepted at
`b97d22e1214266e0f9164e34bb28ab05b4880cc1`; cumulative allocated SUB32 at
`e8f2ae8337314fb20590411eab0c8e519c837df2`; cumulative allocated HUGE8 at
`7396eca6f0115a46829c2fba941a093a77215623`. Historical package and boundary
evidence remain retained.

Non-SHF_ALLOC cumulative `R_386_32`/`R_386_SUB32` is repaired and independently
accepted at `2f1dc83b8e0e38841f2ea291dff236a3f9265e36`. The retained same-offset
fixture changed from incorrect -3 to ABI-required -1. The exact-candidate
two-worker LLD build, focused 1/1 test, selected 10/10 IA-16 LLD regressions,
allocated/nonallocated parity, signed-min/below-min coverage, ordered retained
relocations, hashes, and nonauthor review are retained.

Malformed cases, remaining overflow/retention and other Gate 1 requirements
remain open. No broad integration, emulator runtime, release or external
acceptance.
Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: repair-nonalloc-cumulative-sub32-artifex-v1/evidence under /Users/tedbullock/Developer/llvm-ia16-tasks
Next-Outcome: Remaining Gate 1 SEGELF acceptance after nonallocated SUB32 repair
