# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 8
Verified-At: 2026-09-13T14:43:08Z
Verified-Against: 184a0ab0a84d5d02abe4c9c28dc84b15cdc17b86
Evidence-Layer: source-build-selected-test-object-relocation-scan-and-bounded-linker-partial

Owner: Gate 1 SEGELF matrix partial; protected-mode blocker exposed
Task-System-Status: adopted-v1.0.0
Package-Digest: 0a745fc715ebcd5f665b64c4e9ee9c495371707542594b1a3ad710a4d59a07ce
Gate-1: incomplete; baseline, MC matrix, and SEG16 overflow passed; protected-mode SEGELF rejection fails; remaining matrix pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Acceptance-Status: baseline-pass; G1-MC-pass; G1-SEGELF-partial; SEG16-overflow-pass; protected-mode-blocked; release-not-proven
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Independent-Review: protected-mode signaling/rejection defect confirmed at exact source
Blocker: protected-mode identity is lost before ELF linking, so LLD applies forbidden real-mode SEGELF semantics

At `184a0ab0a84d5d02abe4c9c28dc84b15cdc17b86`, Clang
records `ia16-protected-mode = 1` in IR but warns that `+protected-mode` is
unrecognized. The ELF32 object has `e_flags = 0`, only the generic ABI note,
and one `R_386_SEG16`. LLD exits 0 and writes `0x1234`; ABI v0.2 requires
protected-mode rejection. A nonauthor reviewer independently reproduced the
defect. The accepted real-mode SEGELF and SEG16 overflow tests pass 2/2.

Malformed input, retention, remaining overflow boundaries, and wider semantics
remain unproved. No broad `check-lld`, runtime,
integration, package, release, deployment, or external acceptance is added.

Requirements: `agent-state/ROADMAP.md`
Evidence register: `agent-state/ACCEPTANCE.md`
Capsule: `/Users/tedbullock/Developer/llvm-ia16-tasks/resume-gate-1-segelf-matrix-v1`
Evidence: capsule `evidence/partial-184a0ab0a84d`

Next-Outcome: IA-16 — Repair protected-mode SEGELF signaling and rejection
