# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 20
Verified-At: 2026-09-13T20:55:03Z
Verified-Against: 77e1c078e2f39ed1cce837411065ddd0f9badaed
Evidence-Layer: reviewed-test-object-linker-state

Owner: SEGELF acceptance checkpointed at cumulative HUGE8 defect
Task-System-Status: adopted-artifex-2.0.0
Package-Digest: aec810ecb8a1aa788aa3c0b9de7b164fb73a9eada71de6898284fd0e8c79f3cb
Gate-1: incomplete; cumulative HUGE8 defect
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: allocated cumulative HUGE8 uses stale addend; separate repair required

Standalone SUB32 remains accepted at
`b97d22e1214266e0f9164e34bb28ab05b4880cc1`; its historical software-task 1.1.1
package and boundary evidence remain in ACCEPTANCE and retained capsules.
Cumulative allocated SUB32 is independently accepted at
`e8f2ae8337314fb20590411eab0c8e519c837df2`: signed minimum succeeds, one below
is diagnosed for zero/positive/negative addends; build, 8/8 tests, objects,
bytes, hashes and nonauthor review are retained.

Remaining SEGELF acceptance at `77e1c078e2f39ed1cce837411065ddd0f9badaed`
retained 8/8 LLD regressions and found an allocated cumulative HUGE8 defect:
R_386_8(S=18) then R_386_HUGE8(S=3), A=0, writes 03 instead of ABI-required
05. The identical nonallocated expression writes 05. Standalone HUGE8 controls
pass. Acceptance stopped; malformed, remaining overflow/retention and other
Gate 1 requirements remain open. No repair, integration, runtime or external acceptance is claimed.

Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: remaining-gate1-segelf-after-cumulative-sub32-artifex-v1/evidence under /Users/tedbullock/Developer/llvm-ia16-tasks
Next-Outcome: Repair allocated cumulative HUGE8 addend handling
