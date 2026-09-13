# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 15
Verified-At: 2026-09-13T18:24:39Z
Verified-Against: 1b64773fe8a91f5027ec6dabab92b6e6f0dd3094
Evidence-Layer: management-source-and-capsule-with-retained-product-evidence

Owner: Gate 1 SEGELF acceptance is checkpointed partial; clean custody returns to the existing LLVM owner
Task-System-Status: adopted-v1.1.1
Package-Digest: 8065099ee4a84eaf9aed5caf1dfb4d61554bc619f1522a02957a85176798a50a
Gate-1: incomplete; accepted slices and both SUB16 repairs pass; standalone SUB32 overflow defect confirmed
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: standalone R_386_SUB32 silently wraps A-S = -2147483649 instead of diagnosing signed-32 overflow; repair and remaining matrix require fresh authority

At retained source `1b64773fe8a91f5027ec6dabab92b6e6f0dd3094`, the retained
Gate 1 LLD build (`13993a92f94ea8c0224f737ebcc9458aa0ed565a`) passed the
selected six-test slice (6/6). `R_386_SUB32` and `R_386_HUGE8` records, bounded link,
and relocatable retention were retained; malformed `R_386_HUGE8` with invalid
relocation-section `sh_info` was rejected. A standalone SUB32 lower-bound
fixture proves `S=0x80000001, A=0` links successfully and writes `0x7fffffff`
for unrepresentable `-2147483649`. Independent exact-source review confirms
the LLD defect. No product source changed; broad integration, runtime, release,
packaging, and external effects remain unproved.

Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: resume-remaining-gate-1-segelf-after-nonalloc-v1/evidence/run-1b64773f under the local task directory
Next-Outcome: Await fresh authority for a bounded standalone R_386_SUB32 overflow repair and regression
