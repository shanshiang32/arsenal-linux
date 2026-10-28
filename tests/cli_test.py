#!/usr/bin/env python3
"""End-to-end tests use isolated saves, including EOF and corrupt-file behavior."""
import pathlib
import subprocess
import sys
import tempfile
binary = str(pathlib.Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory(prefix='arsenal-cli-') as directory:
    save = pathlib.Path(directory) / 'game.sav'
    def run(commands, *options):
        return subprocess.run([binary, '--save', str(save), '--no-animation',
                               '--no-color', '--seed', '42', *options],
                              input=commands, text=True, capture_output=True)
    first = run('open 1 2\nlock 1\nsell 1\nsimulate 1 1000\nopen 0\nopen 1 1abc\nquit\n')
    assert first.returncode == 0, first.stderr
    assert '已锁定' in first.stdout and '请先解锁' in first.stdout
    assert '超出允许范围' in first.stdout
    state = save.read_bytes()
    second = run('stats\n')  # EOF saves safely.
    assert second.returncode == 0 and '开箱 2 次' in second.stdout
    assert save.read_bytes() == state
    third = run('simulate 1 100\nquit\n')
    assert third.returncode == 0 and save.read_bytes() == state
    bad = run('', '--seed', '-1')
    assert bad.returncode != 0
    save.write_text('ARSENAL 1\nbroken\n')
    broken = save.read_bytes()
    result = run('open 1\nquit\n')
    assert result.returncode != 0 and save.read_bytes() == broken
print('PASS: CLI persistence, input validation, simulation isolation, corrupt-save protection')
