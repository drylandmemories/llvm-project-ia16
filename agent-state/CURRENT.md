# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 12
Verified-At: 2026-09-13T16:47:37Z
Verified-Against: 77d20218a5ed8e7eccaa8717269c13f447a27860
Evidence-Layer: source-selected-test-object-relocation-scan-and-bounded-linker-partial

Owner: Non-allocated SEGELF pair defect confirmed; repair pending
Task-System-Status: adopted-v1.0.0
Package-Digest: 0a745fc715ebcd5f665b64c4e9ee9c495371707542594b1a3ad710a4d59a07ce
Gate-1: incomplete; accepted slices and SUB16 repair pass; non-allocated pair fails; remaining matrix pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Acceptance-Status: baseline-pass; G1-MC-pass; G1-SEGELF-partial; SEG16-overflow-pass; protected-mode-rejection-pass; SUB16-overflow-pass; nonalloc-pair-fail; release-not-proven
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Independent-Review: nonauthor reproduced c3db and confirmed the cumulative-addend defect
Blocker: non-SHF_ALLOC adjacent R_386_16/R_386_SUB16 does not preserve cumulative ABI semantics; later cases remain unproved

Product relocation sources at `77d20218a5ed8e7eccaa8717269c13f447a27860`
are unchanged from accepted repair `85b57e39dcd46a21179e5b47018757396f29991c`.
Five accepted IA-16 LLD tests passed 5/5 with one worker.

In a retained ELF32 `EM_386` object, initial addend 9 and adjacent
`R_386_16 target` / `R_386_SUB16 base` at offset zero in non-SHF_ALLOC
`.debug_ia16` should produce `0x003d` for target `0x1234` and base `0x1200`.
LLD emits `0xdbc3`. Its generic non-allocated path calls `relocateNoSym` and
loses the cumulative addend expected by X86 SUB16 handling. The stop condition
prevented later matrix execution.

Requirements: `agent-state/ROADMAP.md`
Evidence register: `agent-state/ACCEPTANCE.md`
Capsule: `/Users/tedbullock/Developer/llvm-ia16-tasks/resume-remaining-gate-1-segelf-after-sub16-v1`
Evidence: capsule `evidence/run-77d20218`

Next-Outcome: IA-16 — Repair non-allocated adjacent SEGELF subtraction semantics
