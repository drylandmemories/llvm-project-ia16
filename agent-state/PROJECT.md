# LLVM IA-16 Target

Project-ID: llvm-ia16-target
Repository: https://github.com/drylandmemories/llvm-project-ia16.git
Task-System: artifex
Task-System-Version: 3.1.1

This project owns the IA-16 target implementation, target-specific tests, ABI and code-generation decisions, and evidence for upstream integration. General LLVM or Clang cleanup is outside scope unless a task explicitly admits it.

The GitHub fork is the canonical collaboration repository. The official
`llvm/llvm-project` repository is the read-only upstream source used to update
the fork; upstream synchronization does not authorize publishing IA-16 work to
official LLVM.

## Branch workflow

- `main` is the pristine mirror of upstream LLVM and is updated automatically.
  Do not place IA-16 development commits on it.
- `ia16/development` is the reviewed, accepted IA-16 integration line. Every
  new outcome branch must start from its current commit and use a distinct
  `codex/<outcome>` branch.
- Advance `ia16/development` only by a non-force fast-forward to commits that
  are clean, reviewed, guarded, and accepted for integration. Defect findings,
  partial results, and unreviewed work remain on their outcome branches and
  must not advance the integration line.
- Never force-push or automatically rebase `ia16/development`. Upstream
  synchronization of `main` does not rewrite or rebase development branches.

Each software task owns one bounded outcome in an explicitly owned worktree, preserves unrelated changes, reports evidence at its actual layer, leaves a compact handoff, and retires.
