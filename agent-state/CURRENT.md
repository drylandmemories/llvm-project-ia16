# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 9
Verified-At: 2026-09-13T15:22:50Z
Verified-Against: 954183a34ff7030d40bdc7907bdf6bbc41472a18
Evidence-Layer: source-build-selected-test-object-relocation-scan-and-bounded-linker-partial

Owner: Protected-mode SEGELF repair accepted; remaining Gate 1 matrix pending
Task-System-Status: adopted-v1.0.0
Package-Digest: 0a745fc715ebcd5f665b64c4e9ee9c495371707542594b1a3ad710a4d59a07ce
Gate-1: incomplete; baseline, MC matrix, SEG16 overflow, and protected-mode SEGELF rejection passed; remaining matrix pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Acceptance-Status: baseline-pass; G1-MC-pass; G1-SEGELF-partial; SEG16-overflow-pass; protected-mode-rejection-pass; release-not-proven
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Independent-Review: exact repair candidate accepted with no findings
Blocker: remaining malformed, retention, overflow, and wider relocation semantics are unproved

At `954183a34ff7030d40bdc7907bdf6bbc41472a18`, Clang preserves IA-16 mode
through an additive object note without changing ABI v0.2 or relocation
numbers. LLD validates explicit modes and rejects `R_386_SEG16` in protected
output across allocated and non-allocated paths. Unmarked inputs remain
compatible and non-establishing.

The exact-candidate build completed and six selected Clang/LLD tests pass 6/6.
A protected ELF32 `EM_386` fixture retains ABI 0.2, protected mode, and one
`R_386_SEG16`; LLD exits 1. Its real-mode counterpart links, writes bytes
`34 12`, and retains the mode note. Independent review also exercised the
non-allocated path and accepted overflow boundary.

Requirements: `agent-state/ROADMAP.md`
Evidence register: `agent-state/ACCEPTANCE.md`
Capsule: `/Users/tedbullock/Developer/llvm-ia16-tasks/repair-protected-mode-segelf-v1`
Evidence: capsule `evidence/pass-954183a34ff7`

Next-Outcome: IA-16 — Resume remaining Gate 1 SEGELF matrix after protected-mode repair
