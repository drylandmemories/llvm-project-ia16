# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 13
Verified-At: 2026-09-13T17:19:30Z
Verified-Against: 13993a92f94ea8c0224f737ebcc9458aa0ed565a
Evidence-Layer: source-build-selected-test-object-relocation-scan-and-bounded-linker-partial

Owner: Non-allocated SEGELF pair repair accepted; remaining Gate 1 matrix pending
Task-System-Status: adopted-v1.0.0
Package-Digest: 0a745fc715ebcd5f665b64c4e9ee9c495371707542594b1a3ad710a4d59a07ce
Gate-1: incomplete; accepted slices and both SUB16 repairs pass; remaining matrix pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Acceptance-Status: baseline-pass; G1-MC-pass; G1-SEGELF-partial; SEG16-overflow-pass; protected-mode-rejection-pass; SUB16-overflow-pass; nonalloc-pair-pass; release-not-proven
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Independent-Review: exact candidate accepted after an independent focused rerun
Blocker: R_386_SUB32, R_386_HUGE8, malformed cases, additional overflow, and relocation retention remain unproved

At `13993a92f94ea8c0224f737ebcc9458aa0ed565a`, LLD preserves the implicit REL
addend when applying `R_386_SUB16` in a non-SHF_ALLOC section.
An adjacent `R_386_16 target` / `R_386_SUB16 base` expression with initial
addend 9, target `0x1234`, and base `0x1200` now produces ABI-required
`0x003d` (`3d 00`). Allocated relocation handling, ABI v0.2, and relocation
numbers are unchanged.

The candidate LLD build completed with two workers. The focused regression
passed 1/1 and the accepted IA-16 LLD slice passed 5/5 with one worker. A
retained ELF32 `EM_386` object proves flags-zero `.debug_ia16` and ordered
same-offset relocations; nonauthor review found no issue.

Requirements: `agent-state/ROADMAP.md`
Evidence register: `agent-state/ACCEPTANCE.md`
Capsule: `/Users/tedbullock/Developer/llvm-ia16-tasks/repair-nonalloc-segelf-pair-v1`
Evidence: capsule `evidence/run-13993a92`

Next-Outcome: IA-16 — Resume remaining Gate 1 SEGELF matrix after non-allocated pair repair
