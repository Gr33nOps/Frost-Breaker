"""Check the saved browser projects and the audio the game actually loads."""
import json
from pathlib import Path
import subprocess
import wave
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets/audio-source"


def pcm(path):
    with wave.open(str(path), "rb") as f:
        assert f.getsampwidth() == 2, f"{path}: expected 16-bit PCM"
        rate, channels = f.getframerate(), f.getnchannels()
        samples = np.frombuffer(f.readframes(f.getnframes()), dtype="<i2").reshape(-1, channels)/32768
    return samples, rate


def main():
    manifest = json.loads((SOURCE / "manifest.json").read_text())
    assert len(manifest["music"]) == 2 and len(manifest["sfx"]) == 14
    for name, bars, bpm in [("glacier-run",32,112), ("snowbound-menu",16,84),
                            ("start",1,240), ("level-clear",2,180), ("game-over",2,150)]:
        data = json.loads((SOURCE / f"beepbox/{name}.json").read_text(encoding="utf-8"))
        assert data["loopBars"] == bars and data["beatsPerMinute"] == bpm
        assert all(len(c["sequence"]) == bars for c in data["channels"])
        assert any(p["notes"] for c in data["channels"] for p in c["patterns"]), name

    for item in manifest["sfx"]:
        samples, rate = pcm(ROOT / item["file"])
        assert rate in (44100, 48000) and .015 < len(samples)/rate <= 3.5, item["name"]
        assert .03 < np.sqrt(np.mean(samples**2)) < .25, f"{item['name']}: silent or excessively loud"
        assert np.max(np.abs(samples)) < .8, f"{item['name']}: clipping"
        assert np.max(np.abs(samples.mean(0))) < .003, f"{item['name']}: DC offset"
        assert not np.any(samples[0]) and not np.any(samples[-1]), f"{item['name']}: hard edge"
        assert (SOURCE / item["source"]).is_file(), item["name"]
        if item["name"] == "frost-shatter":
            assert set(item["layers"]) == {"jsfxr/frost-shatter.json", "jsfxr/frost-shatter-ring.json"}
            assert all((SOURCE / layer).is_file() for layer in item["layers"])
    for item in manifest["music"]:
        master, rate = pcm(SOURCE / f"masters/{item['name']}.wav")
        assert rate == 48000 and len(master) == item["frames"]
        assert np.max(np.abs(master.mean(0))) < .0001, f"{item['name']}: DC offset"
        assert np.max(np.abs(master)) < .82
        # Decode the runtime OGG too; an intact WAV alone cannot prove the game asset works.
        raw = subprocess.check_output(["ffmpeg","-hide_banner","-loglevel","error","-i",
            str(ROOT / item["file"]),"-f","f32le","-acodec","pcm_f32le","-"])
        decoded = np.frombuffer(raw,dtype="<f4").reshape(-1,2)
        assert abs(len(decoded)-len(master)) <= 2, f"{item['name']}: loop padding or truncation"
        assert np.max(np.abs(decoded)) < .95, f"{item['name']}: codec overshoot"
        assert np.max(np.abs(decoded[0]-decoded[-1])) < .035, f"{item['name']}: audible seam discontinuity"
        assert np.sqrt(np.mean((decoded[:4800]-master[:4800])**2)) < .02
        print(f"PASS {item['name']}: {len(decoded)/rate:.2f}s decoded stereo loop, clean seam")
    browser_patches = json.loads((SOURCE / "jsfxr/browser-exports.json").read_text())
    assert len(browser_patches) == 12 and all(p["clipped"] == "0" for p in browser_patches)
    print("PASS 5 editable BeepBox projects, 12 jsfxr patches, 14 runtime effects/jingles")


if __name__ == "__main__":
    main()
