#!/bin/sh

set -eu

expected_origin=https://github.com/drylandmemories/llvm-project-ia16.git
expected_upstream=https://github.com/llvm/llvm-project.git

repo_root=$(git rev-parse --show-toplevel)
cd "$repo_root"

origin_url=$(git remote get-url origin)
origin_push_url=$(git remote get-url --push origin)
upstream_url=$(git remote get-url upstream)

if [ "$origin_url" != "$expected_origin" ]; then
  echo "refusing sync: origin is $origin_url" >&2
  exit 1
fi

if [ "$origin_push_url" != "$expected_origin" ]; then
  echo "refusing sync: origin push destination is $origin_push_url" >&2
  exit 1
fi

if [ "$upstream_url" != "$expected_upstream" ]; then
  echo "refusing sync: upstream is $upstream_url" >&2
  exit 1
fi

git_common_dir=$(git rev-parse --git-common-dir)
case "$git_common_dir" in
  /*) ;;
  *) git_common_dir="$repo_root/$git_common_dir" ;;
esac

lock_dir="$git_common_dir/ia16-upstream-sync.lock"
if ! mkdir "$lock_dir" 2>/dev/null; then
  echo "refusing sync: another upstream sync appears to be active" >&2
  exit 1
fi
trap 'rmdir "$lock_dir"' EXIT HUP INT TERM

git fetch --no-tags upstream \
  refs/heads/main:refs/remotes/upstream/main
git fetch --no-tags origin \
  refs/heads/main:refs/remotes/origin/main

if [ "$(git rev-parse refs/remotes/origin/main)" = \
     "$(git rev-parse refs/remotes/upstream/main)" ]; then
  echo "fork main is already synchronized"
  exit 0
fi

if ! git merge-base --is-ancestor \
  refs/remotes/origin/main refs/remotes/upstream/main; then
  echo "refusing sync: fork main is ahead of or diverged from upstream main" >&2
  exit 1
fi

git push origin \
  refs/remotes/upstream/main:refs/heads/main
git fetch --no-tags origin \
  refs/heads/main:refs/remotes/origin/main

if [ "$(git rev-parse refs/remotes/origin/main)" != \
     "$(git rev-parse refs/remotes/upstream/main)" ]; then
  echo "sync verification failed: fork and upstream main differ" >&2
  exit 1
fi

echo "fork main synchronized to $(git rev-parse refs/remotes/upstream/main)"
