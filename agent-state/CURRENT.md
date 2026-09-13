# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 5
Verified-At: 2026-09-13T13:49:30Z
Verified-Against: 6826ec878258c71da4c7355f6bb8544ac56476c1
Evidence-Layer: source-build-selected-test-opcode-scan

Owner: Gate 1 MC matrix complete pending successor custody
Task-System-Status: adopted-v1.0.0
Package-Digest: 0a745fc715ebcd5f665b64c4e9ee9c495371707542594b1a3ad710a4d59a07ce
Gate-1: incomplete; baseline and per-CPU MC matrix passed; other registers pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Acceptance-Status: baseline-pass; G1-MC-pass; release-not-proven
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Independent-Review: pass at exact candidate
Blocker: Gate-1 SEGELF malformed, overflow, and retention matrix incomplete

At `6826ec878258c71da4c7355f6bb8544ac56476c1`, four selected tests passed.
Five CPU fixtures account 73 instructions, 157 executable bytes, and 10 load
regions. Fifty cases reject unavailable families, 386 resources,
FS/GS, and post-286 examples. Scanner cases reject `64` through `67` and
hostile later `0F` group extensions. Injected prefixes are classifier records,
not hostile ELF fixtures.

The baseline at `5d330b7d33eb3fa73035596b82f3c21c1ec843bd` retains its
build, selected-test, opcode-scan, and emulator-runtime evidence. No runtime,
broad integration, package, release, deployment, or external acceptance is
added here.

Requirements: `agent-state/ROADMAP.md`
Evidence register: `agent-state/ACCEPTANCE.md`
Capsule: `/Users/tedbullock/Developer/llvm-ia16-tasks/gate-1-mc-generation-rejection-v1`
Evidence: capsule `evidence/candidate-6826ec878258`

Next-Outcome: IA-16 — Gate 1 SEGELF malformed, overflow, and retention matrix

Calibrate successor reasoning to its capsule: low for narrow repeatable work,
medium for ordinary judgment, and high or xhigh for difficult diagnosis,
architecture, or review. Adjust on observed complexity and record why.
