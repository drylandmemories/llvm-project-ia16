# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 17
Verified-At: 2026-09-13T20:15:19Z
Verified-Against: ebb6fd59f6cb6fb7e292756dace769b46ef3b422
Evidence-Layer: reviewed-project-state-and-retained-source-build-selected-test-object-linker

Owner: Bounded Artifex reconciliation; serial acceptance handoff pending
Task-System-Status: adopted-artifex-2.0.0
Package-Digest: aec810ecb8a1aa788aa3c0b9de7b164fb73a9eada71de6898284fd0e8c79f3cb
Gate-1: incomplete; standalone SUB32 repaired; remaining acceptance pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: remaining Gate 1 matrix lacks complete acceptance evidence

Standalone SUB32 evidence remains bound to
`b97d22e1214266e0f9164e34bb28ab05b4880cc1`: signed-32 A-S with unsigned
ELF32 symbol recovery accepts -2147483648 as 0x80000000 and rejects
-2147483649 with zero/negative addends. Original failing fixture rejects;
7/7 selected LLD regressions, object records, linked bytes, retention and
independent review are retained. This reconciliation adds no compiler proof.
The historical repair used software-task 1.1.1, digest
`8065099ee4a84eaf9aed5caf1dfb4d61554bc619f1522a02957a85176798a50a`.
Cumulative/paired SUB32, HUGE8 and remaining Gate 1 obligations remain open.
No broad integration, emulator runtime or release acceptance is claimed.

Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: repair-standalone-sub32-overflow-v1/evidence under /Users/tedbullock/Developer/llvm-ia16-tasks
Next-Outcome: Remaining Gate 1 SEGELF acceptance after SUB32 repair
