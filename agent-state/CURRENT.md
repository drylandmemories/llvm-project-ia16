# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 25
Verified-At: 2026-09-13T22:15:27Z
Verified-Against: 08dbcfa107579d813fb2dabb724a4fe321d3c053
Evidence-Layer: reviewed-source-build-selected-test-malformed-input-repair

Owner: accepted malformed SEGELF bounds repair; paused at Ted request
Task-System-Status: adopted-artifex-2.0.0
Package-Digest: aec810ecb8a1aa788aa3c0b9de7b164fb73a9eada71de6898284fd0e8c79f3cb
Gate-1: incomplete; malformed SEGELF bounds repair accepted; remaining requirements pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: none for the completed repair; broader work intentionally paused

Prior accepted SEGELF milestones remain recorded in the evidence register.

Malformed relocation-field bounds are independently accepted at
`08dbcfa107579d813fb2dabb724a4fe321d3c053`. An i386-only early pass
rejects `R_386_SEG16`, `R_386_HUGE8`, `R_386_SUB16`, and `R_386_SUB32` when
their fields do not fit the target section. The exact-candidate build passed;
allocated and non-SHF_ALLOC final links, `-r`, GC section-symbol, `.eh_frame`,
partial-tail, and very-large-offset cases fail closed; and the ten retained
IA-16 LLD regressions plus the new test pass 11/11 with two workers. Nonauthor
review accepted the exact candidate after rejecting and correcting earlier
validation bypasses.

Other malformed cases, retention proof, and other Gate 1 requirements remain
open. No broad integration, emulator runtime, release, or external acceptance.
Ted requested a pause after this repair's acceptance to preserve usage; no
successor was dispatched.
Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: `/Users/tedbullock/Developer/llvm-ia16-tasks/repair-malformed-segelf-bounds-artifex-v1/evidence`
Next-Outcome: On Ted resume, continue the remaining SEGELF acceptance matrix from this accepted repair
