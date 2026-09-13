# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 22
Verified-At: 2026-09-13T21:21:55Z
Verified-Against: 6df8fb8774f6181e3f3efa45a812a30b0d257c81
Evidence-Layer: reviewed-test-object-linker-defect-checkpoint

Owner: Nonallocated cumulative SUB32 defect checkpoint; bounded repair next
Task-System-Status: adopted-artifex-2.0.0
Package-Digest: aec810ecb8a1aa788aa3c0b9de7b164fb73a9eada71de6898284fd0e8c79f3cb
Gate-1: incomplete; SEGELF repair required before remaining foundation acceptance
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: non-SHF_ALLOC cumulative R_386_SUB32 loses the current implicit addend

Standalone SUB32 is accepted at
`b97d22e1214266e0f9164e34bb28ab05b4880cc1`; cumulative allocated SUB32 at
`e8f2ae8337314fb20590411eab0c8e519c837df2`; cumulative allocated HUGE8 at
`7396eca6f0115a46829c2fba941a093a77215623`. Historical package and boundary
evidence remain retained.

Remaining SEGELF acceptance stopped at a reproducible nonallocated cumulative
SUB32 defect at clean source `6df8fb8774f6181e3f3efa45a812a30b0d257c81`.
An ELF32 REL `.debug_ia16` fixture with same-offset `R_386_32 first` followed
by `R_386_SUB32 second`, with first=1 and second=2, retains both relocations but
writes `fdffffff` (-3). The frozen cumulative ABI requires `ffffffff` (-1).
Independent inspection confirms `InputSection::relocateNonAlloc` routes SUB32
through `relocateNoSym`, discarding the current implicit addend. No repository
source or tests changed in the acceptance task.

Malformed cases, remaining overflow/retention and other Gate 1 requirements
remain open. No integration, emulator runtime, release or external acceptance.
Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: remaining-gate1-segelf-after-huge8-artifex-v1/evidence under /Users/tedbullock/Developer/llvm-ia16-tasks
Next-Outcome: Repair nonallocated cumulative R_386_SUB32
