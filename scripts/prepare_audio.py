"""Master browser-exported BeepBox/jsfxr WAVs and install the game audio.

Does not synthesize music or effects. Keep browser-exports untouched so the
original exports and their editable projects remain available.
"""
import argparse
import json
from pathlib import Path
import subprocess
import wave
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets/audio-source"
EXPORTS = SOURCE / "browser-exports"
MASTERS = SOURCE / "masters"
SOUNDS = ROOT / "assets/sounds"
SFX_NAMES = ["wall-tick", "paddle-knock", "frost-shatter", "rock-hit", "rock-break",
             "lava-ignite", "life-lost", "menu-back", "pause", "resume", "launch",
             "start", "level-clear", "game-over"]


def read_wav(path):
    with wave.open(str(path), "rb") as f:
        assert f.getsampwidth() == 2 and f.getnchannels() in (1, 2), path
        rate, channels = f.getframerate(), f.getnchannels()
        samples = np.frombuffer(f.readframes(f.getnframes()), dtype="<i2").astype(np.float64)
    return samples.reshape(-1, channels) / 32768, rate


def write_wav(path, samples, rate):
    path.parent.mkdir(parents=True, exist_ok=True)
    assert np.max(np.abs(samples)) < .99, path
    with wave.open(str(path), "wb") as f:
        f.setnchannels(samples.shape[1])
        f.setsampwidth(2)
        f.setframerate(rate)
        f.writeframes(np.round(samples * 32767).astype("<i2").tobytes())


def normalize(samples, target=.14, ceiling=.79):
    rms = np.sqrt(np.mean(samples ** 2))
    return samples * min(target / max(rms, 1e-9), ceiling / np.max(np.abs(samples)))


def ogg(source, target):
    target.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(["ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-i", str(source),
                    "-c:a", "libvorbis", "-q:a", "5", str(target)], check=True)


def prepare_effect(name):
    samples, rate = read_wav(EXPORTS / f"{name}.wav")
    source = f"{'beepbox' if name in ('start', 'level-clear', 'game-over') else 'jsfxr'}/{name}.json"
    layers = [source]
    if name == "frost-shatter":
        ring, ring_rate = read_wav(EXPORTS / "frost-shatter-ring.wav")
        assert rate == ring_rate and samples.shape[1] == ring.shape[1]
        crack = normalize(samples, .14, .60)
        ring = normalize(ring, .14, .60)
        delay = round(rate * .006)
        samples = np.zeros((max(len(crack), delay + len(ring)), crack.shape[1]))
        samples[:len(crack)] += crack
        samples[delay:delay + len(ring)] += ring * .20
        layers.append("jsfxr/frost-shatter-ring.json")
    samples -= np.mean(samples, axis=0)
    samples = normalize(samples, .12 if name == "frost-shatter" else .16,
                         .62 if name == "frost-shatter" else .72)
    attack, release = round(rate * .0008), min(round(rate * .006), len(samples) // 5)
    window = np.ones(len(samples))
    window[:attack] = np.linspace(0, 1, attack)
    window[-release:] = np.linspace(1, 0, release)
    samples *= window[:, None]
    samples -= window[:, None] * (samples.sum(axis=0) / window.sum())
    write_wav(SOUNDS / "sfx" / f"{name}.wav", samples, rate)
    return dict(name=name, seconds=len(samples) / rate,
                file=f"assets/sounds/sfx/{name}.wav", source=source, layers=layers)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--sfx", choices=SFX_NAMES)
    args = parser.parse_args()
    manifest = {"tools": {"music": "https://www.beepbox.co/", "sfx": "https://sfxr.me/"},
                "sample_origin": "Original scores and patches rendered by the browser editors",
                "music": [], "sfx": []}
    if args.sfx:
        manifest = json.loads((SOURCE / "manifest.json").read_text())
    else:
        for name, bpm, bars in [("glacier-run", 112, 32), ("snowbound-menu", 84, 16)]:
            raw, rate = read_wav(EXPORTS / f"{name}.wav")
            expected = round(rate * 60 / bpm * 4 * bars)
            assert abs(len(raw) - 2 * expected) <= 2, "Export exactly two loop repetitions"
            # The second repetition carries the first repetition's reverb/chorus tail.
            # Export duration is rounded once by BeepBox, so split at its rounded bar boundary.
            samples = raw[expected:expected * 2].copy()
            samples -= np.mean(samples, axis=0)
            samples = normalize(samples, .10 if name == "snowbound-menu" else .14)
            master = MASTERS / f"{name}.wav"
            write_wav(master, samples, rate)
            ogg(master, SOUNDS / "music" / f"{name}.ogg")
            manifest["music"].append(dict(name=name, bpm=bpm, bars=bars, sample_rate=rate,
                frames=len(samples), seconds=len(samples)/rate,
                file=f"assets/sounds/music/{name}.ogg", source=f"beepbox/{name}.json"))

    effects = {item["name"]: item for item in manifest["sfx"]}
    for name in [args.sfx] if args.sfx else SFX_NAMES:
        effects[name] = prepare_effect(name)
    manifest["sfx"] = list(effects.values())

    # Short listening reel: eight seconds of each theme, then the important impacts.
    reel = []
    for name in ["snowbound-menu", "glacier-run"]:
        samples, rate = read_wav(MASTERS / f"{name}.wav")
        samples = samples[:8*rate].copy() * .75
        n = rate//5
        samples[:n] *= np.linspace(0, 1, n)[:, None]
        samples[-n:] *= np.linspace(1, 0, n)[:, None]
        reel += [samples, np.zeros((rate//3, 2))]
    for name in ["wall-tick", "paddle-knock", "frost-shatter", "rock-hit", "rock-break", "lava-ignite", "level-clear"]:
        samples, sample_rate = read_wav(SOUNDS / "sfx" / f"{name}.wav")
        if sample_rate != rate:
            indices = np.arange(round(len(samples)*rate/sample_rate)) * sample_rate/rate
            samples = np.stack([np.interp(indices, np.arange(len(samples)), samples[:, c])
                                for c in range(samples.shape[1])], axis=1)
        if samples.shape[1] == 1:
            samples = np.repeat(samples, 2, axis=1)
        reel += [samples*.8, np.zeros((rate//3, 2))]
    preview = SOURCE / "previews/audio-preview.wav"
    write_wav(preview, np.concatenate(reel), rate)
    ogg(preview, SOURCE / "previews/audio-preview.ogg")
    (SOURCE / "manifest.json").write_text(json.dumps(manifest, indent=2)+"\n", encoding="utf-8")
    print(f"Installed {args.sfx}." if args.sfx else
          "Installed 2 looping OGG tracks and 14 WAV effects/jingles from browser exports.")


if __name__ == "__main__":
    main()
