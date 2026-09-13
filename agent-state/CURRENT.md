# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 11
Verified-At: 2026-09-13T16:19:46Z
Verified-Against: 85b57e39dcd46a21179e5b47018757396f29991c
Evidence-Layer: source-build-selected-test-object-relocation-scan-and-bounded-linker-partial

Owner: Standalone R_386_SUB16 overflow repair accepted; remaining Gate 1 matrix pending
Task-System-Status: adopted-v1.0.0
Package-Digest: 0a745fc715ebcd5f665b64c4e9ee9c495371707542594b1a3ad710a4d59a07ce
Gate-1: incomplete; prior accepted slices and standalone SUB16 overflow repair passed; remaining matrix pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Acceptance-Status: baseline-pass; G1-MC-pass; G1-SEGELF-partial; SEG16-overflow-pass; protected-mode-rejection-pass; SUB16-overflow-pass; release-not-proven
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Independent-Review: exact repair candidate accepted with no findings after a distinct 5/5 selected-test run
Blocker: remaining paired/non-allocated expression, malformed, retention, and wider relocation semantics are unproved

At `85b57e39dcd46a21179e5b47018757396f29991c`, LLD computes standalone
`R_386_SUB16` as `A-S`, diagnoses results outside signed 16-bit range, and
retains the separate adjacent `R_386_16`/`R_386_SUB16` exception path. ABI v0.2
and relocation numbers are unchanged.

The exact-candidate LLD build completed. Author and independent reviewer each
passed the five selected IA-16 LLD tests 5/5. A retained ELF32 `EM_386` object
contains standalone SUB16 relocations at offsets 0 and 2; `-32768` writes
`00 80`, while `-32769` is rejected and produces no output ELF.

Requirements: `agent-state/ROADMAP.md`
Evidence register: `agent-state/ACCEPTANCE.md`
Capsule: `/Users/tedbullock/Developer/llvm-ia16-tasks/repair-standalone-sub16-overflow-v1`
Evidence: capsule `evidence/pass-85b57e39dcd4`

Next-Outcome: IA-16 — Resume remaining Gate 1 SEGELF matrix after standalone SUB16 repair
