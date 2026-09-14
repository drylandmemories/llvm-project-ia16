# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 26
Verified-At: 2026-09-14T01:26:11Z
Verified-Against: 08dbcfa107579d813fb2dabb724a4fe321d3c053
Evidence-Layer: reviewed-source-build-selected-test-and-management-state

Owner: Artifex 3.1.1 local repair accepted; compiler work paused by Ted
Task-System-Status: adopted-artifex-3.1.1
Package-Digest: 2712dc55b4fdc3428e4ae13c8be55eca174f81663e7923ec5a059f59ca90316a
Gate-1: incomplete; malformed SEGELF bounds repair accepted; remaining requirements pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: none; broader product work paused

Malformed relocation-field bounds are independently accepted at
`08dbcfa107579d813fb2dabb724a4fe321d3c053`. An i386-only early pass
rejects `R_386_SEG16`, `R_386_HUGE8`, `R_386_SUB16`, and `R_386_SUB32` when
their fields do not fit the target section. The exact-candidate build passed;
allocated and non-SHF_ALLOC final links, `-r`, GC section-symbol, `.eh_frame`,
partial-tail, and very-large-offset cases fail closed; and the ten retained
IA-16 LLD regressions plus the new test pass 11/11 with two workers. Nonauthor
review accepted the exact candidate after rejecting and correcting earlier
validation bypasses.

Artifex 3.1.1 metadata was reconciled at
`58a5af20b2abdca9216a36d77e3a8d28082dde6d`; admission-readiness only.

Other malformed cases, retention proof, and other Gate 1 requirements remain
open. No broad integration, emulator runtime, release, or external acceptance.
Ted requested a pause after this repair's acceptance to preserve usage; no
successor was dispatched.
Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: `/Users/tedbullock/Developer/llvm-ia16-tasks/repair-malformed-segelf-bounds-artifex-v1/evidence`
Next-Outcome: On Ted resume, continue the remaining SEGELF acceptance matrix from this accepted repair
