# Current project state

Schema: 1
Project-ID: llvm-ia16-target
Revision: 14
Verified-At: 2026-09-13T18:04:39Z
Verified-Against: e02b4efd182537f3f5f8bc7b407bc2510cfb5f51
Evidence-Layer: management-source-and-capsule-with-retained-product-evidence

Owner: Adoption complete; clean custody returns to existing LLVM owner
Task-System-Status: adopted-v1.1.1
Package-Digest: 8065099ee4a84eaf9aed5caf1dfb4d61554bc619f1522a02957a85176798a50a
Gate-1: incomplete; accepted slices and both SUB16 repairs pass; remaining matrix pending
Gate-2: incomplete
Gate-3: incomplete
Gate-4: incomplete
Gate-5: incomplete
Execution-Mode: serial-same-checkout-two-workers-one-heavy-process
Blocker: R_386_SUB32, R_386_HUGE8, malformed cases, additional overflow and relocation retention remain unproved

Retained product source13993a92f94ea8c0224f737ebcc9458aa0ed565a fixes the
non-allocated adjacent SEGELF SUB16 implicit addend; the retained REL object
produces ABI-required0x003d. Allocated handling, ABI0.2 and relocation numbers
are unchanged. Its LLD build, focused1/1 and accepted5/5 tests passed, with
independent exact-source acceptance. Earlier baseline/MC, SEG16-overflow,
protected-mode rejection and SUB16-overflow acceptances remain; release and
emulator runtime are not proved by this adoption.

1.1.1 pins/guidance and fresh capsule validation passed independent review.
Old capsules/packages and custody receipts remain. No product source/test,
heavy process or successor launch in adoption. Ted authorized resumption after
the budget hold; further roadmap work remains with its existing owner.

Requirements: agent-state/ROADMAP.md
Evidence register: agent-state/ACCEPTANCE.md
Product evidence: repair-nonalloc-segelf-pair-v1/evidence/run-13993a92 under the local task directory
Next-Outcome: IA-16 — Resume remaining Gate 1 SEGELF matrix after non-allocated pair repair
