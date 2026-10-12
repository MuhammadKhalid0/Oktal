#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./compare_task5.sh                 # compares origin/task5 vs all origin/* branches
#   ./compare_task5.sh origin/master   # compares origin/master vs all origin/* branches
#
# Optional env vars:
#   SHOW_STAT=1  -> include `git diff --stat` for each comparison
#   SHOW_LOG=1   -> include `git log --oneline --left-right` for each comparison

BASE="${1:-origin/task5}"

SHOW_STAT="${SHOW_STAT:-0}"
SHOW_LOG="${SHOW_LOG:-0}"

# Update remote refs (recommended)
git fetch --all --prune >/dev/null 2>&1 || true

# Verify base exists as a remote-tracking branch (refs/remotes/...) or local branch (refs/heads/...)
if ! git show-ref --verify --quiet "refs/remotes/${BASE}" \
   && ! git show-ref --verify --quiet "refs/heads/${BASE}"; then
  echo "Error: base branch '$BASE' not found."
  echo "Hint: try one of these:"
  echo "  git branch -r | grep -F '${BASE#origin/}'"
  echo "  git branch | grep -F '${BASE}'"
  exit 1
fi

echo "Comparing all origin/* branches against: $BASE"
echo

# Collect origin remote branches except HEAD and BASE itself
mapfile -t BRANCHES < <(
  git for-each-ref --format='%(refname:short)' refs/remotes/origin |
    grep -v '^origin/HEAD$' |
    grep -v "^${BASE}$" |
    sort
)

if [ "${#BRANCHES[@]}" -eq 0 ]; then
  echo "No other origin branches found to compare."
  exit 0
fi

printf "%-35s %12s %14s\n" "branch" "only_in_base" "only_in_branch"
printf "%-35s %12s %14s\n" "-----" "------------" "--------------"

for b in "${BRANCHES[@]}"; do
  counts="$(git rev-list --left-right --count "${BASE}...${b}")"
  left="$(awk '{print $1}' <<<"$counts")"
  right="$(awk '{print $2}' <<<"$counts")"
  printf "%-35s %12s %14s\n" "$b" "$left" "$right"

  if [ "$SHOW_STAT" = "1" ]; then
    echo "  -- diff --stat (${BASE}...${b})"
    git diff --stat "${BASE}...${b}" | sed 's/^/  /' || true
  fi

  if [ "$SHOW_LOG" = "1" ]; then
    echo "  -- commits (${BASE}...${b})"
    git log --oneline --left-right "${BASE}...${b}" | sed 's/^/  /' || true
  fi

  if [ "$SHOW_STAT" = "1" ] || [ "$SHOW_LOG" = "1" ]; then
    echo
  fi
done
