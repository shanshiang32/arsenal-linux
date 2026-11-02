#!/usr/bin/env bash
# Debian 13 cloud fallback without root; official signed indexes, no system installation.
set -euo pipefail
base=/workspace/qt6-local
mkdir -p "$base/apt/lists/partial" "$base/apt/cache/archives/partial" "$base/root"
printf 'deb https://deb.debian.org/debian trixie main\n' > "$base/apt/sources.list"
opts=(-o Debug::NoLocking=1 -o "Dir::Etc::sourcelist=$base/apt/sources.list" -o Dir::Etc::sourceparts=- -o "Dir::State::lists=$base/apt/lists" -o "Dir::Cache=$base/apt/cache")
apt-get "${opts[@]}" update
apt-get "${opts[@]}" --download-only -y install qt6-base-dev qt6-base-dev-tools fonts-noto-cjk
for package in "$base"/apt/cache/archives/*.deb; do dpkg-deb -x "$package" "$base/root"; done
python3 - <<'PY'
from pathlib import Path
r=Path('/workspace/qt6-local/root')
for p in (r/'usr/lib/x86_64-linux-gnu/pkgconfig').glob('*.pc'):
 p.write_text(p.read_text().replace('prefix=/usr','prefix='+str(r/'usr')))
for p in r.rglob('*'):
 if p.is_symlink() and not p.exists():
  q=Path('/usr/lib/x86_64-linux-gnu')/p.readlink().name
  if q.exists(): p.unlink();p.symlink_to(q.resolve())
PY
cd /workspace/arsenal-linux
source tools/cloud-env.sh
make check-deps
make -j4 all test
