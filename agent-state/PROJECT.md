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

Each software task owns one bounded outcome in an explicitly owned worktree, preserves unrelated changes, reports evidence at its actual layer, leaves a compact handoff, and retires.
