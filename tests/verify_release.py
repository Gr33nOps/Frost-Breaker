import json
import hashlib
import re
import zipfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
version = json.loads((root / 'package.json').read_text())['version']

with zipfile.ZipFile(root / f'dist/frost-breaker-{version}-web.zip') as archive:
    names = set(archive.namelist())
    assert archive.testzip() is None
    assert 'index.html' in names
    for path in ['main.js', 'engine.js', 'view.js', 'audio.js', 'style.css', 'levels.json',
                 'assets/fonts/Jersey10-Regular.ttf', 'assets/fonts/OFL.txt', 'LICENSE', 'THIRD_PARTY.md']:
        assert path in names, path
    assert len(json.loads(archive.read('levels.json'))) == 10
    for name in names:
        assert not name.startswith(('/', '\\')) and '..' not in Path(name).parts
        assert not name.lower().endswith(('.exe', '.dll', '.pdb', '.aseprite', '.piskel', '.zip'))
    for name in ['main.js', 'audio.js', 'view.js', 'style.css', 'index.html']:
        source = archive.read(name).decode()
        assert not re.search(r'https?://', source), name
    assert archive.getinfo('assets/images/logo.png').file_size > 0
    assert sum(entry.file_size for entry in archive.infolist()) < 30_000_000
print('PASS: browser ZIP has a root index, ten levels, local resources, licenses, and no development files')

windows = root / f'dist/frost-breaker-{version}-windows.zip'
if windows.exists():
    with zipfile.ZipFile(windows) as archive:
        assert archive.testzip() is None
        names = set(archive.namelist())
        for path in ['Play.cmd', 'bin/Frost Breaker.exe', 'bin/sfml-graphics-2.dll', 'bin/sfml-window-2.dll',
                     'bin/sfml-system-2.dll', 'bin/sfml-audio-2.dll', 'bin/openal32.dll', 'bin/vcruntime140.dll',
                     'bin/msvcp140.dll', 'data/highscore.txt', 'LICENSE', 'licenses/SFML.md', 'assets/fonts/OFL.txt']:
            assert path in names, path
        assert 'licenses/OpenAL-Soft-1.18.1-source.tar.gz' in names
        assert 'licenses/OpenAL-Soft-COPYING.txt' in names
        assert archive.read('data/highscore.txt').strip() == b'0'
        assert not any(name.lower().endswith(('.obj', '.pdb', '.ilk')) for name in names)
    print('PASS: Windows ZIP includes executable, runtime DLLs, fresh save, artwork, audio, and licenses')

for line in (root / 'dist/SHA256SUMS.txt').read_text().splitlines():
    expected, name = line.split('  ', 1)
    assert hashlib.sha256((root / 'dist' / name).read_bytes()).hexdigest() == expected, name
print('PASS: all release archive checksums match')
