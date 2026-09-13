# Disposable Task Manager adoption

Status: **Artifex 2.0.0 pinned for fresh capsules**

## Immutable release

- Package: `/Users/tedbullock/Developer/Artifex/releases/2.0.0/artifex`
- Digest: `aec810ecb8a1aa788aa3c0b9de7b164fb73a9eada71de6898284fd0e8c79f3cb`
- Entry point: `$artifex`; validator: `scripts/artifex.py` in that package.
- Adoption identity: schema 2, system `artifex`, stable project `llvm-ia16-target`,
  exact origin `https://github.com/llvm/llvm-project.git`.

Historical v1 capsules continue using `$software-task` and their original
immutable packages, including
`/Users/tedbullock/Developer/CodexTaskSystem/releases/1.1.1/software-task`
(digest `8065099ee4a84eaf9aed5caf1dfb4d61554bc619f1522a02957a85176798a50a`).
Keep all historical capsules and releases unchanged, including partial and
superseded tasks. A migration does not authorize resuming their product work.
The Git-common ledger `software-task-state.json` and corresponding lock keep
their existing names: these are the Artifex 2.0 compatibility ABI.
CURRENT remains the revisioned product-evidence snapshot; its historical
package fields do not override the active pin in `task-system.json` and PROJECT.

Use this versioned script; the global skill is only an entry point. Never copy, fork, or vendor the package.

Existing capsules retain their original packages and inputs. Adopt only at a
clean task boundary; never repin a running capsule. Preflight with
`init --dry-run`, then use ordinary admission: preflight grants no custody.
Read the package's sizing and GitHub references at creation/action time.
One useful result includes its focused verification; repository size or task
count alone does not justify splitting. Select model and effort separately
from actual uncertainty, preserve user choices, and confirm launch settings.
Environment failures require environment diagnosis, not automatic escalation.
Verify GitHub host/repository against PROJECT, configuration and origin, then
authenticated visibility before any separately authorized write. The upstream
origin is identity evidence, not permission to publish to upstream LLVM.
Resolve the canonical native project before dispatch; queued creation is not
custody. Preserve SERIAL_WORKFLOW receipts, resource limits and all gates.

## Local interface and operation

`AGENTS.md`, `agent-state/*`, and the one-shot adoption owner are repository-owned. Keep capsules outside the checkout.

Preserve active work. Start one bounded Feature or Bug from an owned clean worktree and exact commit. Record scope, exclusions, authorization, fixture, finish, and review. The fresh task reads and validates its capsule. Finish or checkpoint truthfully, publish reviewed facts to CURRENT, hand off, and retire.

Keep evidence layers separate. The CLI does not dispatch tasks, integrate, deploy, or authorize consequences.

Invoke the adoption owner only for one bounded repair or migration; preserve compiler work, validate, record, and retire.
