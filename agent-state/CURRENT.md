# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 19
Verified-At: 2026-09-13T20:43:07Z
Verified-Against: e8f2ae8337314fb20590411eab0c8e519c837df2
Evidence-Layer: reviewed-source-build-selected-test-object-linker-and-project-state

Owner: cumulative SUB32 repair complete; SEGELF acceptance handoff pending
Task-System-Status: adopted-artifex-2.0.0
Package-Digest: aec810ecb8a1aa788aa3c0b9de7b164fb73a9eada71de6898284fd0e8c79f3cb
Gate-1: incomplete; standalone and cumulative SUB32 repaired; remaining acceptance pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: remaining SEGELF and other Gate 1 requirements lack complete acceptance evidence

Standalone SUB32 evidence remains accepted at
`b97d22e1214266e0f9164e34bb28ab05b4880cc1`: signed-32 A-S with unsigned
ELF32 symbol recovery accepts -2147483648 and rejects -2147483649 with
zero/negative addends. Its historical repair used software-task 1.1.1,
digest `8065099ee4a84eaf9aed5caf1dfb4d61554bc619f1522a02957a85176798a50a`.

Cumulative allocated R_386_32/SUB32 overflow is repaired and independently
accepted at `e8f2ae8337314fb20590411eab0c8e519c837df2`. Its regression covers zero,
positive and negative cumulative addends:
-2147483648 is written as 0x80000000 and -2147483649 is diagnosed before
truncation. Exact-candidate build, 1/1 and 8/8 selected tests, ELF32 records,
bytes, hashes and nonauthor review are retained.

R_386_HUGE8, malformed cases, remaining relocation retention/overflow, and
all other Gate 1 obligations remain open. No broad integration, emulator
runtime, release, publication or deployment acceptance is claimed.

Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: repair-cumulative-sub32-overflow-artifex-v1/evidence under /Users/tedbullock/Developer/llvm-ia16-tasks
Next-Outcome: Remaining Gate 1 SEGELF acceptance after cumulative SUB32 repair
