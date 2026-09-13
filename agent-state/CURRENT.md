# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 21
Verified-At: 2026-09-13T21:05:49Z
Verified-Against: 7396eca6f0115a46829c2fba941a093a77215623
Evidence-Layer: reviewed-source-build-test-object-linker-state

Owner: Cumulative HUGE8 repair complete; remaining SEGELF acceptance next
Task-System-Status: adopted-artifex-2.0.0
Package-Digest: aec810ecb8a1aa788aa3c0b9de7b164fb73a9eada71de6898284fd0e8c79f3cb
Gate-1: incomplete; remaining SEGELF and foundation acceptance
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: none for remaining bounded SEGELF acceptance

Standalone SUB32 remains accepted at
`b97d22e1214266e0f9164e34bb28ab05b4880cc1`; cumulative allocated SUB32 at
`e8f2ae8337314fb20590411eab0c8e519c837df2`. Historical package and boundary
evidence remain in ACCEPTANCE and retained capsules.

Allocated cumulative HUGE8 is independently accepted at
`7396eca6f0115a46829c2fba941a093a77215623`. The symbolic regression failed before repair:
R_386_8(S=18) then R_386_HUGE8(S=3), A=0 wrote 03 instead of 05.
The repaired path reads the preceding relocated byte; allocated and nonallocated
controls now agree. Retained exact-candidate build, 9/9 LLD regressions, ELF32
objects, linked bytes, ordered retention and nonauthor review cover signed
initial addends, a three-relocation chain, standalone controls, and preceding
R_386_8 overflow at 256 and -129. This is bounded repair acceptance only.

Malformed cases, remaining overflow/retention and other Gate 1 requirements
remain open. No integration, emulator runtime, release or external acceptance.
Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: repair-cumulative-huge8-artifex-v1/evidence under /Users/tedbullock/Developer/llvm-ia16-tasks
Next-Outcome: Resume remaining Gate 1 SEGELF acceptance
