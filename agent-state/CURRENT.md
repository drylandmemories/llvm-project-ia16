# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 4
Verified-At: 2026-09-13T13:26:35Z
Verified-Against: 5d330b7d33eb3fa73035596b82f3c21c1ec843bd
Evidence-Layer: source-build-test-opcode-scan-emulator-runtime

Owner: reproducible baseline complete pending successor custody
Task-System-Status: adopted-v1.0.0
Package-Digest: 0a745fc715ebcd5f665b64c4e9ee9c495371707542594b1a3ad710a4d59a07ce
Roadmap-Status: gates-1-through-5-preserved
Gate-1: incomplete; baseline passed; comprehensive matrices pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Overall-Progress: approximately-20-percent-rough-unverified-estimate
Acceptance-Status: baseline-pass; release-not-proven
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Independent-Review: pass at exact candidate
Blocker: Gate-1 per-CPU MC generation-and-rejection matrix incomplete

The baseline built its tools with two workers; passed 7 LLVM, 18 Clang, and 2
LLD lit tests including SEGELF; and passed the exact 7 TargetParser tests.

Nine fresh arithmetic, division-edge, and byte-swap probes passed on 8086,
80186, and 80286 DOSBox-X at `-O0`, `-O2`, and `-Os`. All 3,395 instructions
and load regions were accounted. Selection drift and extra executable sections
fail closed. Mounted names were only `PROBE.COM` and `RESULT.OK`.

Requirements: `agent-state/ROADMAP.md`
Evidence register: `agent-state/ACCEPTANCE.md`
Capsule: `/Users/tedbullock/Developer/llvm-ia16-tasks/reproducible-baseline-acceptance-v1`
Evidence: capsule `evidence/candidate-5d330b7d33eb`

No full Gate 1, broad integration, differential, package, release, deployment,
or external acceptance is claimed.

Next-Outcome: IA-16 — Gate 1 per-CPU MC generation and rejection matrix

Use the configured default model at low reasoning unless complexity justifies
escalation. Preserve uppercase DOS 8.3 mounted names. Historical DOS fixture
name auditing is a later concern, not completed here.
