# LLVM IA-16 serial task workflow

Why: A hard reset followed concurrent memory-heavy work. IA-16 builds,
regression suites, linkers, and emulators must therefore run with explicit
resource and ownership limits while still advancing the complete roadmap.

Scope: Use one disposable software task at a time in the same local checkout,
with no standing manager or background team. This governs project work; it does
not weaken any Gate 1–5 acceptance requirement.

## Admission

1. Read `AGENTS.md`, `agent-state/PROJECT.md`, `agent-state/CURRENT.md`, and the
   task-relevant portions of `ROADMAP.md`, `ACCEPTANCE.md`, and this workflow.
2. Inspect the exact checkout, full HEAD, checkpoint ancestry, Git status,
   current capsule/state ledger, locks, journals, build processes, and emulator
   processes. Existing dirt belongs to its creating task. Stop rather than
   absorb, reset, reformat, stage, or commit it.
3. Admit one bounded outcome with explicit acceptance layer, fixture,
   authorization, observable finish, exclusions, and stop condition. Keep the
   original assignment complete. Separate authority for this task's product
   writes from still-valid authority to dispatch the next bounded task. A
   narrow capsule does not cancel an explicit user instruction to progress
   through sequential roadmap outcomes.
4. Initialize its capsule under
   `/Users/tedbullock/Developer/llvm-ia16-tasks` from the clean exact source
   using the immutable package pinned in `task-system.json`. Read `startup.md`
   fully and validate before product work.

## Execution and resource ceiling

- Use the same `/Users/tedbullock/Developer/llvm-project-ia16` checkout; tasks
  are serial for every write and heavy process. The only overlap is read-only
  custody confirmation: after reviewed disposition, state commit, handoff, and
  clean custody, the predecessor stops all writes and launches exactly one
  local successor. That successor reads and validates its capsule, inspects
  task/process/repository state, and reports custody acceptance. Only then does
  the successor own writes and the predecessor retire.
- Use no more than two build/test workers (`-j2`, or the tool's equivalent).
- Run only one heavy process at a time: one compiler/linker build, one broad
  lit/check suite, one randomized campaign, one opcode scan campaign, or one
  emulator instance. Lightweight source inspection and log parsing may occur
  only when they do not materially increase memory pressure.
- Prefer the existing external build directory and retained caches after
  verifying identity. Do not start a duplicate build merely because output
  observation timed out.
- If interrupted or reset, inspect Git/source identity, process handles,
  capsule and shared locks, pending journals, output timestamps, and external
  effects before continuing. Never replay an uncertain link, state write,
  deployment, or emulator acceptance run as though it had not occurred.
- If task creation or delivery is uncertain, inspect the local task list and
  exact capsule/task identity before retrying. Never create a second successor
  merely because the first creation response was lost or delayed.
- After two materially equivalent failures, change the diagnostic method.

## Task writing contract

- Every authored assignment and handoff opens with separate `Why:` and `Scope:`
  paragraphs. Preserve all user intent; these paragraphs orient rather than
  replace the unchanged assignment.
- Every project commit body opens with `Why:` and `Scope:` paragraphs and adds
  a `Validation:` paragraph naming exact commands/results and evidence layer.
  A title alone or a passing test without scope is insufficient.
- Product commits contain only owned hunks. Management commits contain only
  repository task-state/workflow paths. Do not combine them.
- Record source, build, test, integration, opcode-scan, emulator runtime, and
  package evidence separately and bind each to the exact full commit.

## Review, state, and retirement

1. Run focused checks first, then the widest justified check while respecting
   the one-heavy-process rule. Persist exact commands, configs, logs, hashes,
   and artifacts in the task directory.
2. Obtain independent review whenever the contract requires it. Review binds
   the exact candidate diff/source and cannot be inferred from a name in JSON.
3. Finish or checkpoint the capsule truthfully. A partial result stays partial;
   a management test does not prove compiler behavior, and a build does not
   prove emulator runtime.
4. Prepare a CURRENT candidate from the actual current revision. Publish it
   only after semantic review, with an exact one-step revision increment and
   the capsule's guarded state operation. Commit/read back through repository
   policy and reconcile the shared ledger when required.
5. Decide product disposition and retirement separately. `Next-Outcome` is a
   factual route, not authority. Re-read the current task's unchanged original
   assignment and any capsule-attached continuation authority. If Ted directed
   the project to advance through sequential bounded outcomes and that
   authority still covers the recorded next outcome, do not invent an
   `AWAITING TED` stop merely because the just-finished capsule was narrower.
   Carry the exact authority source and conditions into the successor capsule.
   If authority truly does not cover continuation, keep the completed task
   visible and resumable as awaiting-user; do not claim retirement.
6. Leave a compact handoff with outcome, exact commits/paths, checks, retained
   resources, unresolved blocker, and next safe action. Before dispatch, stop
   every checkout/build/test/emulator writer and require clean exact committed
   custody. Create exactly one successor and bind its capsule digest and a
   dispatch nonce to the returned task or client identity. Native creation is
   not idempotent: inspect uncertain delivery and never blindly create another.
7. The successor starts product-read-only. It validates capsule, source,
   CURRENT/ledger, locks, journals, dirt and processes, then records immutable
   capsule-local `READY.json`. The predecessor rechecks the freeze and records
   immutable `RELEASED.json`; release is irreversible after interruption. The
   successor repeats the checks and records immutable `ACKNOWLEDGED.json`.
   Only acknowledgement permits successor product writes and predecessor
   retirement. Each receipt binds transition/stage/time/actor, task and capsule
   identities/digests, dispatch nonce and returned identity, repository/source,
   CURRENT/package, authority, observed processes/state and write permission.
   The pinned CLI does not mechanically enforce these sidecars.
8. Every final outcome reports its gate, roadmap progress, evidence at each
   actual layer, unresolved blocker, exact successor and receipt disposition.
   Missing applicable receipt stages block only a `RETIRED` claim, not a
   truthful completed/held/blocked/awaiting-user report. Natural terminal
   completion with no authorized continuation needs no successor. No later
   task may depend on conversation history.

Routine clean transfer is checklist work suitable for a calibrated lighter
model. Escalate reasoning for ambiguous authority, uncertain/duplicate
dispatch, custody or state/process mismatch, and post-release recovery; do not
add a permanent high-reasoning manager.

## Dependency order

Run regressions before expanding capability. Complete tasks serially in this
order: foundation closure (ABI/triples/parser/MC/SEGELF); near code and data
correctness; segmented Clang C and explicit far behavior; hosted DOS COM/MZ;
multiple segments plus 80286 protected mode; then huge pointers, full
runtime/sysroot, optimization hardening, release suites, and packaging.

## Serial opening task

The first successor is `IA-16 — Reproducible baseline acceptance`. Existing
compiler capabilities need repeatable acceptance because the historical
25-test slice did not establish the full parser/relocation boundary and its
runtime probes were not one reproducible candidate suite. Scope permits
repository-owned acceptance harnesses and tests, but not compiler/linker/runtime
implementation repairs.

The task reproduces the current build and test baseline; explicitly includes
SEGELF and target-parser unit tests; and exercises arithmetic, signed/unsigned
division edges, and 16-bit byte swaps at `-O0`, `-O2`, and `-Os`. It uses
deterministic fresh directories and bounded emulator runs; records full
source/tool/CPU identities; accounts for opcode coverage; maps every admitted
requirement to evidence; and identifies one bounded next gap. An implementation
defect is handed off as a separate task.

Its capsule must not be initialized until this migration has passed independent
review, CURRENT revision 3 is committed, and the shared ledger is reconciled.
