# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 18
Verified-At: 2026-09-13T20:29:10Z
Verified-Against: c0be9ecb4a446380d5bfe7d06b57d7d2557d7019
Evidence-Layer: reviewed-project-state-and-retained-source-build-selected-test-object-linker

Owner: SEGELF acceptance checkpoint; cumulative SUB32 repair handoff pending
Task-System-Status: adopted-artifex-2.0.0
Package-Digest: aec810ecb8a1aa788aa3c0b9de7b164fb73a9eada71de6898284fd0e8c79f3cb
Gate-1: incomplete; standalone SUB32 repaired; remaining acceptance pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: cumulative SUB32 silently wraps signed overflow; separate repair required

Standalone SUB32 evidence at `b97d22e1214266e0f9164e34bb28ab05b4880cc1`
accepts -2147483648 and rejects -2147483649 with zero/negative addends.
Selected tests, objects, linked bytes, retention and review are retained.
The historical repair used software-task 1.1.1, digest
`8065099ee4a84eaf9aed5caf1dfb4d61554bc619f1522a02957a85176798a50a`.
Cumulative/paired SUB32, HUGE8 and remaining Gate 1 obligations remain open.
No broad integration, emulator runtime or release acceptance is claimed.

Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: repair-standalone-sub32-overflow-v1/evidence under /Users/tedbullock/Developer/llvm-ia16-tasks
Acceptance checkpoint at c0be9ecb4a446380d5bfe7d06b57d7d2557d7019:
verified retained tool hashes and unchanged compiler source; 7/7 LLD regressions
pass. Adjacent allocated R_386_32/SUB32 with A=0, zero=0, target=0x80000001
incorrectly succeeds with 0x7fffffff instead of diagnosing -2147483649.
The -2147483648 boundary succeeds correctly. No implementation changed.
Evidence: gate1-segelf-after-sub32-artifex-v1/evidence under the same task root.
Next-Outcome: Repair cumulative SUB32 signed overflow, then resume SEGELF acceptance
