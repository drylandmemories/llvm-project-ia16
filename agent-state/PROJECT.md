# LLVM IA-16 disposable task project

Project-ID: llvm-ia16
Repository: https://github.com/llvm/llvm-project.git
Task-System-Version: 1.0.0

Use the shared `software-task` package at
`/Users/tedbullock/.agents/skills/software-task`, pinned by
`agent-state/task-system.json`. Keep each capsule outside the source checkout.

Only IA-16 implementation, tests, ABI and code-generation decisions, and their
upstream-integration evidence belong to this project. General upstream LLVM or
Clang cleanup is excluded unless a task expressly admits it. Preserve unrelated
dirty work and keep source, build, test, integration, and runtime evidence
separate.
