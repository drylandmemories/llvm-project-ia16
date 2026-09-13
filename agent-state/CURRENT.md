# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 24
Verified-At: 2026-09-13T21:48:32Z
Verified-Against: 6c237ad5076d65afa69b9072dd9c3c451c5f76a0
Evidence-Layer: reviewed-test-object-linker-malformed-input-defect-checkpoint

Owner: malformed SEGELF relocation bounds defect checkpoint; bounded repair next
Task-System-Status: adopted-artifex-2.0.0
Package-Digest: aec810ecb8a1aa788aa3c0b9de7b164fb73a9eada71de6898284fd0e8c79f3cb
Gate-1: incomplete; SEGELF malformed-input repair required before foundation acceptance
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: out-of-range IA-16 SEGELF relocation fields are read and written without bounds rejection

Standalone SUB32 remains accepted at `b97d22e1214266e0f9164e34bb28ab05b4880cc1`;
allocated SUB32 at `e8f2ae8337314fb20590411eab0c8e519c837df2`; cumulative
HUGE8 at `7396eca6f0115a46829c2fba941a093a77215623`; and non-SHF_ALLOC
SUB32 at `2f1dc83b8e0e38841f2ea291dff236a3f9265e36`.

Remaining SEGELF acceptance at source
`6c237ad5076d65afa69b9072dd9c3c451c5f76a0` found a reviewed malformed-input
defect. Exact-current `yaml2obj` fixtures place `R_386_SEG16`, `R_386_HUGE8`,
`R_386_SUB16`, or `R_386_SUB32` at the end of its target section. All four link
and emit output instead of being rejected. Nonauthor reproduction and source
inspection confirm unchecked type-width reads and writes. Reserved type 48
fails closed with an extra internal-linker-error diagnostic. No source or test
changed in this acceptance task.

Other malformed cases, retention proof, and other Gate 1 requirements
remain open. No broad integration, emulator runtime, release, or external
acceptance.
Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: remaining-gate1-segelf-after-nonalloc-sub32-artifex-v1/evidence under /Users/tedbullock/Developer/llvm-ia16-tasks
Next-Outcome: Reject malformed IA-16 SEGELF relocation fields before access
