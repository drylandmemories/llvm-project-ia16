# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 6
Verified-At: 2026-09-13T14:01:26Z
Verified-Against: 18edd2802535a1feaba86d03c8ef4e8f04517e94
Evidence-Layer: source-selected-test-object-relocation-scan-and-bounded-linker-defect

Owner: Gate 1 SEGELF matrix partial; bounded overflow repair pending
Task-System-Status: adopted-v1.0.0
Package-Digest: 0a745fc715ebcd5f665b64c4e9ee9c495371707542594b1a3ad710a4d59a07ce
Gate-1: incomplete; baseline and per-CPU MC matrix passed; SEGELF overflow blocked
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Acceptance-Status: baseline-pass; G1-MC-pass; G1-SEGELF-partial; release-not-proven
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Independent-Review: partial at exact source; SEG16 implementation defect confirmed
Blocker: R_386_SEG16 output overflow is silently truncated rather than diagnosed

At `18edd2802535a1feaba86d03c8ef4e8f04517e94`, the existing selected SEGELF
test passed. A minimal object contains one `R_386_SEG16` relocation at offset
zero. Linking with `S=0x100000` should diagnose the unrepresentable paragraph
value `0x10000`, but the retained LLD build exits successfully and writes
`0x0000`. A nonauthor reviewer independently reproduced and confirmed the
implementation defect. The capsule forbids its repair, so the broader
malformed, retention, protected-mode rejection, and remaining overflow matrix
was not expanded in this task.

The baseline at `5d330b7d33eb3fa73035596b82f3c21c1ec843bd` retains its
build, selected-test, opcode-scan, and emulator-runtime evidence. No runtime,
broad integration, package, release, deployment, or external acceptance is
added here.

Requirements: `agent-state/ROADMAP.md`
Evidence register: `agent-state/ACCEPTANCE.md`
Capsule: `/Users/tedbullock/Developer/llvm-ia16-tasks/gate-1-segelf-matrix-v1`
Evidence: capsule `evidence/partial-18edd2802535`

Next-Outcome: IA-16 — Diagnose R_386_SEG16 output overflow
