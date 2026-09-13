# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 16
Verified-At: 2026-09-13T18:52:26Z
Verified-Against: b97d22e1214266e0f9164e34bb28ab05b4880cc1
Evidence-Layer: source-build-selected-test-object-linker-and-independent-review

Owner: Bounded standalone SUB32 overflow repair complete; no successor authorized
Task-System-Status: adopted-v1.1.1
Package-Digest: 8065099ee4a84eaf9aed5caf1dfb4d61554bc619f1522a02957a85176798a50a
Gate-1: incomplete; standalone SUB32 signed overflow repaired; remaining acceptance matrix pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: remaining Gate 1 acceptance matrix and broader roadmap evidence remain incomplete; continuation requires fresh authority

At source `b97d22e1214266e0f9164e34bb28ab05b4880cc1`, standalone
R_386_SUB32 checks signed-32 A-S after recovering the unsigned ELF32 symbol
address. The new regression failed before the fix and passes afterward:
-2147483648 links to 0x80000000, while -2147483649 is rejected, including
zero and negative implicit addends. The original retained failing fixture now
produces the expected overflow diagnostic. The retained Gate 1 linker rebuilt
with at most two workers; the new regression and six accepted LLD tests pass
7/7 at the exact candidate. Object records, linked bytes, relocation retention,
commands and tool hashes are retained. Independent exact-candidate review is
recorded in the task evidence. Cumulative SUB32 handling, HUGE8, ABI v0.2 and
relocation numbers are unchanged. Paired-SUB32 range policy remains outside
this repair; no broad integration, emulator runtime or release proof is claimed.

Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: repair-standalone-sub32-overflow-v1/evidence under /Users/tedbullock/Developer/llvm-ia16-tasks
Next-Outcome: Await fresh authority for remaining Gate 1 acceptance; no successor dispatched
