# LLVM IA-16 project guidance

## Project boundary

- Keep work necessary for the IA-16 target, its tests, and upstream integration here. Unrelated upstream LLVM/Clang cleanup is out of scope.
- Existing dirty files and active branches belong to their creating task. Inspect status; never overwrite, absorb, reset, or reformat their changes.

## Disposable task management

- Use one fresh Codex task per outcome. Work directly; use at most one narrow, temporary specialist when materially required.
- Start from an owned clean worktree and full commit. Read `agent-state/{PROJECT.md,CURRENT.md}` and initialize with the immutable package pinned by `task-system.json`.
- Give the task only its capsule path. It reads `startup.md` fully and validates before work. Keep capsules outside the checkout.
- No permanent managers, standing teams, recursive delegation, or dependency on old conversations. Finish with durable state and a compact handoff, then retire.
- The project-local adoption owner is `.codex/agents/disposable-task-adoption-owner.toml`. Invoke it only for a bounded migration or repair of this project's task-management layer.
- Consume the pinned shared Feature/Bug skill and capsule tooling; never copy, fork, vendor, or recreate it here.
- Read `.codex/DISPOSABLE_TASK_MANAGER_ADOPTION.md` before changing this local integration.

## State and evidence

- CURRENT is reviewed factual state, not a work log or authority. Update it only through the capsule workflow; reconcile concurrent revisions.
- Separate source, build, test, integration, and runtime evidence. Do not promote one layer into another.
- Compiler tests do not authorize integration, deployment, or external consequences.
- Handoffs name the outcome, owned paths, validation, blocker, and next safe action.
