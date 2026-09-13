# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 7
Verified-At: 2026-09-13T14:19:54Z
Verified-Against: 0af18ce9830252ddf97f4a68c0d2947b1af4cd02
Evidence-Layer: source-build-selected-test-object-relocation-scan-and-bounded-linker-pass

Owner: Gate 1 SEGELF matrix partial; overflow blocker repaired
Task-System-Status: adopted-v1.0.0
Package-Digest: 0a745fc715ebcd5f665b64c4e9ee9c495371707542594b1a3ad710a4d59a07ce
Gate-1: incomplete; baseline, per-CPU MC matrix, and bounded SEG16 overflow repair passed; remaining SEGELF matrix pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Acceptance-Status: baseline-pass; G1-MC-pass; G1-SEGELF-partial; SEG16-overflow-pass; release-not-proven
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Independent-Review: pass at exact source; no findings for bounded SEG16 overflow repair
Blocker: remaining SEGELF malformed, retention, protected-mode rejection, and relocation matrix unproved

At `0af18ce9830252ddf97f4a68c0d2947b1af4cd02`, LLD checks the shifted
`R_386_SEG16` paragraph value before its 16-bit write. The focused object has
one `R_386_SEG16` relocation; `S=0xfffff` links to `0xffff`, while
`S=0x100000` exits 1 and diagnoses paragraph value `65536` as out of range.
The existing adjacent same-offset `R_386_16`/`R_386_SUB16` exception test and
the new overflow test pass 2/2. A nonauthor reviewer independently accepted
the exact candidate with no findings.

The baseline at `5d330b7d33eb3fa73035596b82f3c21c1ec843bd` retains its
build, selected-test, opcode-scan, and emulator-runtime evidence. No broad
`check-lld`, runtime, integration, package, release, deployment, or external
acceptance is added here.

Requirements: `agent-state/ROADMAP.md`
Evidence register: `agent-state/ACCEPTANCE.md`
Capsule: `/Users/tedbullock/Developer/llvm-ia16-tasks/r386-seg16-overflow-v2`
Evidence: capsule `evidence/pass-0af18ce98302`

Next-Outcome: IA-16 — Resume bounded Gate 1 SEGELF matrix
