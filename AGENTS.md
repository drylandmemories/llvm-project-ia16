# LLVM IA-16 project guidance

## Project boundary

- Keep work necessary for the IA-16 target, its tests, and upstream integration here. Unrelated upstream LLVM/Clang cleanup is out of scope.
- Existing dirty files and active branches belong to their creating task. Inspect status; never overwrite, absorb, reset, or reformat their changes.

## Disposable task management

- Use one fresh Codex task per outcome. Work directly; use at most one narrow, temporary specialist when materially required.
- Start from an owned clean worktree and full commit. Read `agent-state/{PROJECT.md,CURRENT.md}` and initialize with the immutable package pinned by `task-system.json`.
- Give the task only its capsule path. It reads `startup.md` fully and validates before work. Keep capsules outside the checkout.
- No permanent managers, standing teams, recursive delegation, or dependency on old conversations. Finish with durable state and a compact handoff, then retire.
- A bounded capsule limits the current task's writes; it does not erase a broader, still-valid user instruction to advance this roadmap through sequential disposable tasks. Before ending, distinguish terminal completion from a required successor, preserve the exact continuation authority in that successor's capsule, and complete the project handoff protocol in `agent-state/SERIAL_WORKFLOW.md`.
- The project-local adoption owner is `.codex/agents/disposable-task-adoption-owner.toml`. Invoke it only for a bounded migration or repair of this project's task-management layer.
<!-- artifex:begin -->
Use $artifex for new tasks. Resolve the immutable package pinned by agent-state/task-system.json.
Preserve historical capsules and their original packages.
<!-- artifex:end -->
Never copy, fork, vendor, or recreate the shared packages here.
- Read `.codex/DISPOSABLE_TASK_MANAGER_ADOPTION.md` before changing this local integration.

## State and evidence

- CURRENT is reviewed factual state, not a work log or authority. Update it only through the capsule workflow; reconcile concurrent revisions.
- Separate source, build, test, integration, and runtime evidence. Do not promote one layer into another.
- Compiler tests do not authorize integration, deployment, or external consequences.
- Handoffs name the outcome, owned paths, validation, blocker, and next safe action.
