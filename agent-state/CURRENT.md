# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 10
Verified-At: 2026-09-13T15:37:32Z
Verified-Against: 869d9f3b7ba77585e1a5c5858b2d6404d810983b
Evidence-Layer: source-build-identity-object-relocation-scan-and-bounded-linker-partial

Owner: Standalone R_386_SUB16 overflow defect confirmed; remaining Gate 1 matrix held
Task-System-Status: adopted-v1.0.0
Package-Digest: 0a745fc715ebcd5f665b64c4e9ee9c495371707542594b1a3ad710a4d59a07ce
Gate-1: incomplete; prior accepted slices remain passed; standalone SUB16 overflow defect blocks remaining matrix
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Acceptance-Status: baseline-pass; G1-MC-pass; G1-SEGELF-partial; SEG16-overflow-pass; protected-mode-rejection-pass; SUB16-overflow-defect; release-not-proven
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Independent-Review: exact-source standalone SUB16 defect reproduced and confirmed
Blocker: standalone R_386_SUB16 silently wraps an out-of-range negative result; later malformed, retention, overflow, and wider relocation semantics remain unproved

At `869d9f3b7ba77585e1a5c5858b2d6404d810983b`, an ELF32 `EM_386`
fixture retained standalone `R_386_SUB16` relocations. LLD wrote `0x8000` for
`A-S=-32768`, but returned success and wrapped `A-S=-32769` to `0x7fff`.
ABI v0.2 requires overflow diagnosis outside the adjacent
`R_386_16`/`R_386_SUB16` exception. Independent review reproduced the defect.

The task stopped at that first defect. Paired/non-allocated expressions,
`R_386_SUB32`, `R_386_HUGE8`, and the malformed/retention matrix remain
unproved. No product source, ABI, integration, runtime, or release claim changed.

Requirements: `agent-state/ROADMAP.md`
Evidence register: `agent-state/ACCEPTANCE.md`
Capsule: `/Users/tedbullock/Developer/llvm-ia16-tasks/resume-remaining-gate-1-segelf-matrix-v1`
Evidence: capsule `evidence/partial-869d9f3b7ba7`

Next-Outcome: IA-16 — Repair standalone R_386_SUB16 overflow handling
