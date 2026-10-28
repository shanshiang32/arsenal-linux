#!/usr/bin/env bash
set -euo pipefail
binary=${1:-./build/release/arsenal}
session=$(mktemp -d)
trap 'rm -rf "$session"' EXIT
"$binary" --save "$session/demo.sav" --seed 2026 --no-animation --no-color <<'COMMANDS'
help
cases
daily
open 2 5
bag price
lock 1
sell 1
sell-low
stats
achievements
simulate 3 100000
quit
COMMANDS
