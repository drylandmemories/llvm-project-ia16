# Disposable Task Manager adoption

Status: **shared package pinned; capsule validation pending**

## Scope

This repository will use Ted's shared, versioned Disposable Task Manager package for generic Feature/Bug task setup and capsule initialization/validation. This repository owns only LLVM IA-16-specific boundaries, durable state, and handoffs.

The shared package is owned by Codex task `01a0993f-75bf-7492-b4fd-91b4ecd034c3` and is published as:

- version: `1.0.0`
- location: `/Users/tedbullock/.agents/skills/software-task`
- package digest: `0a745fc715ebcd5f665b64c4e9ee9c495371707542594b1a3ad710a4d59a07ce`

The machine-readable pin is `agent-state/task-system.json`. Project-specific
task boundaries and current factual state live in `agent-state/PROJECT.md` and
`agent-state/CURRENT.md`; generic task behavior remains owned by the shared
package and is not copied into this repository.

## Activation contract

Invoke `.codex/agents/disposable-task-adoption-owner.toml` once the shared package is published, or for one bounded repair of this local integration. The owner must:

1. Preserve all active dirty work and identify ownership before edits.
2. Bind to the published shared package without copying its generic implementation.
3. Add only the project-local state/capsule paths required by the published contract.
4. Validate configuration and capsule behavior without modifying LLVM/Clang product files.
5. Record a compact handoff and retire; no future task may depend on its conversation history.

## Project boundary

Local durable guidance must distinguish:

- IA-16 target implementation, tests, ABI/code-generation decisions, and upstream-integration evidence owned by this project.
- General LLVM/Clang behavior and unrelated upstream cleanup, which are outside a task unless explicitly admitted.
- Source, build, test, integration, and runtime evidence, which remain separate acceptance layers.

## Completion gate

Adoption is complete only when the shared package version/location is recorded, its initializer and validator pass in a disposable test capsule, project-local state is within the shared size/startup limits, and the migration task leaves a bounded durable handoff before retiring.
