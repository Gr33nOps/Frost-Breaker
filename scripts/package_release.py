import argparse
import hashlib
import json
import re
import shutil
import tempfile
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DIST = ROOT / 'dist'
VERSION = json.loads((ROOT / 'package.json').read_text())['version']
IMAGES = ['background', 'menu background', 'logo', 'logo2', 'Ball', 'LavaBall', 'Paddle', 'Brick',
          'Rock', 'LavaPickup', 'overlay', 'start', 'start2', 'Quit', 'quit2', 'resume', 'resume2',
          'menu', 'menu2', 'play again', 'play again2', 'heart', 'PixelFont']


def copy(source, destination):
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, destination)


def resources(destination):
    for name in IMAGES + [f'{i} lives' for i in range(1, 6)]:
        copy(ROOT / f'assets/images/{name}.png', destination / f'assets/images/{name}.png')
    for source in (ROOT / 'assets/animations/png').glob('*.png'):
        copy(source, destination / 'assets/animations/png' / source.name)
    for source in (ROOT / 'assets/sounds').rglob('*'):
        if source.suffix in ['.ogg', '.wav'] and source.name != 'rock-break.wav':
            copy(source, destination / source.relative_to(ROOT))
    for name in ['Jersey10-Regular.ttf', 'OFL.txt']:
        copy(ROOT / 'assets/fonts' / name, destination / 'assets/fonts' / name)
    for name in ['LICENSE', 'THIRD_PARTY.md']:
        copy(ROOT / name, destination / name)


def archive(folder, name):
    target = DIST / f'frost-breaker-{VERSION}-{name}.zip'
    with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as output:
        for source in sorted(folder.rglob('*')):
            if source.is_file():
                output.write(source, source.relative_to(folder).as_posix())
    print(f'{target.name}: {target.stat().st_size / 1048576:.2f} MiB')
    return target


def web(folder):
    resources(folder)
    for source in (ROOT / 'web').iterdir():
        if source.is_file():
            copy(source, folder / source.name)
    text = (ROOT / 'src/Levels.cpp').read_text()
    stages = re.findall(r'\{"([^"]+)",\s*"([^"]+)",\s*\{\{([\s\S]*?)\}\}\}', text)
    levels = [dict(name=name, story=story, tiles=re.findall(r'"([.FRL]+)"', tiles))
              for name, story, tiles in stages]
    if len(levels) != 10 or any(not 1 <= sum(row.count('L') for row in level['tiles']) <= 3 for level in levels):
        raise ValueError('Expected ten levels with one to three lava bricks each')
    (folder / 'levels.json').write_text(json.dumps(levels, indent=2) + '\n')
    copy(ROOT / 'docs/media/icon.png', folder / 'assets/icon.png')
    shutil.copytree(folder, DIST / 'web', dirs_exist_ok=True)
    return archive(folder, 'web')


def windows(folder):
    resources(folder)
    copy(ROOT / 'bin/Frost Breaker.exe', folder / 'bin/Frost Breaker.exe')
    for name in ['sfml-graphics-2.dll', 'sfml-window-2.dll', 'sfml-system-2.dll', 'sfml-audio-2.dll', 'openal32.dll']:
        copy(ROOT / 'third_party/SFML-2.6.1/bin' / name, folder / 'bin' / name)
    redist = Path('C:/Program Files/Microsoft Visual Studio')
    candidates = sorted(redist.glob('*/*/VC/Redist/MSVC/*/x86/Microsoft.VC*.CRT'))
    if not candidates:
        raise FileNotFoundError('Install the Visual Studio C++ redistributable files before packaging Windows')
    for source in candidates[-1].glob('*.dll'):
        copy(source, folder / 'bin' / source.name)
    copy(ROOT / 'Play.cmd', folder / 'Play.cmd')
    copy(ROOT / 'third_party/SFML-2.6.1/license.md', folder / 'licenses/SFML.md')
    for source in (ROOT / 'third_party/licenses').iterdir():
        if source.is_file():
            copy(source, folder / 'licenses' / source.name)
    (folder / 'data').mkdir()
    (folder / 'data/highscore.txt').write_text('0\n')
    (folder / 'README.txt').write_text(
        'FROST BREAKER\n\nExtract the whole ZIP, then double-click Play.cmd.\n'
        'Keep bin, assets and data together. Windows 10/11, x86 application.\n\n'
        'Mouse or arrows: move. Click or Space: launch. Escape: pause.\n'
        'Break frost. Rock stays. Each lava brick adds 10 seconds of piercing time.\n'
        'Levels have 1-3 lava bricks. Lava pauses with play and clears on a lost life or new level.\n'
        'Clear ten levels with five shared lives.\n\n'
        'Source, credits and updates: https://github.com/Gr33nOps/Frost-Breaker\n', encoding='utf-8')
    return archive(folder, 'windows')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--web-only', action='store_true')
    args = parser.parse_args()
    DIST.mkdir(exist_ok=True)
    (ROOT / 'build').mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='release-', dir=ROOT / 'build') as temporary:
        staging = Path(temporary)
        browser = staging / 'web'
        browser.mkdir()
        outputs = [web(browser)]
        if not args.web_only:
            native = staging / 'windows'
            native.mkdir()
            outputs.append(windows(native))
            press = staging / 'press-kit'
            shutil.copytree(ROOT / 'docs/media', press)
            copy(ROOT / 'docs/itch-page.md', press / 'itch-page.md')
            copy(ROOT / 'docs/itch-upload.md', press / 'itch-upload.md')
            copy(ROOT / 'LICENSE', press / 'LICENSE')
            outputs.append(archive(press, 'press-kit'))
            art = staging / 'pixel-art-source'
            art.mkdir()
            for directory, extension in [('assets/aseprite', '*'), ('assets/animations/aseprite', '*.aseprite'),
                    ('assets/animations/native', '*.png'), ('assets/animations/png', '*.png'),
                    ('assets/manual-redraw/piskel', '*.piskel'), ('assets/manual-redraw/png', '*.png'),
                    ('assets/branding', '*')]:
                for source in (ROOT / directory).glob(extension):
                    if source.is_file():
                        copy(source, art / source.relative_to(ROOT))
            copy(ROOT / 'assets/animations/manifest.json', art / 'assets/animations/manifest.json')
            copy(ROOT / 'LICENSE', art / 'LICENSE')
            copy(ROOT / 'THIRD_PARTY.md', art / 'THIRD_PARTY.md')
            (art / 'README.txt').write_text('Frost Breaker editable pixel art\n\n'
                'Open .aseprite files in Aseprite and .piskel files in Piskel.\n'
                'Sources are native pixel size; game exports use nearest-neighbor 2x scaling.\n'
                'Ball 10x10; paddle 40x11; brick 19x9; backgrounds 400x300;\n'
                'logo 290x52; buttons 112x34; overlay 260x300; heart 15x13.\n'
                'The icon uses a 64x64 canvas. Animation manifests give frame durations.\n')
            outputs.append(archive(art, 'pixel-art-source'))
    (DIST / 'SHA256SUMS.txt').write_text(''.join(
        f'{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}\n' for path in outputs))


if __name__ == '__main__':
    main()
