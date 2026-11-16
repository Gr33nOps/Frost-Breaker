"""Author the original Frost Breaker score and precise jsfxr patches.

These are editable projects, not audio renderers. Import the JSON files into
BeepBox/jsfxr and use their browser Export/Download controls for the WAV masters.
"""
from copy import deepcopy
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets/audio-source"


def save(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def note(pitch, start, duration, volume=85, end_volume=None):
    return {"pitches": pitch if isinstance(pitch, list) else [pitch],
            "points": [{"tick": start, "pitchBend": 0, "volume": volume},
                       {"tick": start + duration, "pitchBend": 0,
                        "volume": volume if end_volume is None else end_volume}],
            "continuesLastPattern": False}


def channel(instrument, bars, octave=3, kind="pitch"):
    return {"type": kind, "instruments": [instrument],
            "patterns": [{"notes": sorted(bar, key=lambda n: n["points"][0]["tick"])} for bar in bars],
            "sequence": list(range(1, len(bars) + 1)), "octaveScrollBar": octave}


def song(bpm, bars, channels):
    return {"format": "BeepBox", "version": 9, "scale": "normal :(", "key": "E",
            "introBars": 0, "loopBars": bars, "beatsPerBar": 4, "ticksPerBeat": 4,
            "beatsPerMinute": bpm, "layeredInstruments": False,
            "patternInstruments": False, "channels": channels}


def compose():
    bank = json.loads((SOURCE / "instrument-bank.json").read_text(encoding="utf-8"))
    bell, bass, pad, drums = [deepcopy(c["instruments"][0]) for c in bank["channels"]]
    bell.update(volume=64, effects=["note filter", "reverb"], reverb=20,
                fadeOutTicks=24, feedbackAmplitude=1)
    bass.update(volume=52, fadeOutTicks=-3)
    pad.update(volume=28, chorus=35, fadeOutTicks=24)
    arp = deepcopy(bass)
    arp.update(volume=36, fadeOutTicks=-6)
    drums.update(volume=38, effects=[], reverb=0)

    # E minor(add9), Cmaj7, Gmaj7, Dadd9; two contrasting eight-bar phrases.
    roots = [24, 24, 20, 20, 27, 27, 22, 22]
    chords = [[36, 43, 50, 55], [36, 43, 50, 55],
              [32, 39, 43, 50], [32, 39, 43, 50],
              [31, 38, 43, 50], [31, 38, 43, 50],
              [34, 41, 46, 50], [34, 41, 46, 50]]
    # (tick, relative pitch above E4, duration): deliberate rests leave room for SFX.
    melody_a = [
        [(0, 7, 2), (3, 10, 2), (6, 14, 4), (12, 10, 3)],
        [(0, 7, 3), (4, 3, 3), (8, 2, 2), (11, 0, 4)],
        [(0, 3, 3), (4, 7, 2), (7, 10, 3), (12, 7, 3)],
        [(0, 14, 4), (6, 10, 2), (9, 7, 2), (12, 3, 3)],
        [(0, 10, 3), (4, 14, 2), (7, 19, 4), (13, 14, 2)],
        [(0, 15, 3), (4, 14, 2), (7, 10, 3), (12, 7, 3)],
        [(0, 9, 3), (4, 14, 3), (8, 10, 2), (11, 9, 3)],
        [(0, 7, 3), (4, 2, 3), (8, 2, 2), (12, 7, 2)],
    ]
    melody_b = [
        [(0, 14, 3), (4, 19, 3), (8, 15, 2), (11, 14, 3)],
        [(0, 10, 4), (6, 7, 2), (9, 3, 2), (12, 7, 3)],
        [(0, 15, 3), (4, 14, 2), (7, 10, 3), (12, 7, 3)],
        [(0, 3, 4), (6, 7, 2), (9, 10, 2), (12, 14, 3)],
        [(0, 19, 4), (6, 14, 2), (9, 10, 2), (12, 7, 3)],
        [(0, 10, 3), (4, 14, 3), (8, 15, 2), (12, 14, 3)],
        [(0, 9, 3), (4, 14, 2), (7, 10, 3), (12, 9, 3)],
        [(0, 7, 3), (4, 2, 3), (8, 2, 2), (12, 7, 2)],
    ]
    leads, lows, harmony, arps, rhythm = [], [], [], [], []
    for b in range(32):
        phrase = melody_a if b < 8 or 16 <= b < 24 else melody_b
        leads.append([note(48+p, t, d, 80 if t else 94, 50) for t, p, d in phrase[b % 8]])
        root = roots[b % 8]
        lows.append([note(root, 0, 3, 90, 65), note(root+7, 6, 2, 65, 45),
                     note(root, 8, 3, 80, 55), note(root+12, 14, 1, 55, 35)])
        harmony.append([note(chords[b % 8], 0, 15, 60, 38)])
        pattern = chords[b % 8]
        arps.append([note(pattern[i % 4]+12, i*2+1, 1, 55 if i % 2 else 72, 28)
                     for i in range(8)] if b >= 4 else [])
        beat = [note(0, 0, 1, 72, 20), note(0, 8, 1, 62, 15),
                note(4, 4, 1, 48, 10), note(4, 12, 1, 48, 10)]
        beat += [note(8, t, 1, 22 if t % 4 else 32, 0) for t in [2, 6, 10, 14]]
        if b % 8 == 7:
            beat += [note(5, 15, 1, 30, 0)]
        # A quieter middle eight creates relief before the final phrase.
        rhythm.append(beat if not 16 <= b < 20 else [note(8, 2, 1, 24, 0)])
    gameplay = song(112, 32, [channel(bell, leads, 4), channel(arp, arps, 3),
                             channel(bass, lows, 1), channel(pad, harmony, 2),
                             channel(drums, rhythm, 0, "drum")])
    save(SOURCE / "beepbox/glacier-run.json", gameplay)

    # Same motif at a gentler pace, without a drum kit, for the title screen.
    menu_bell, menu_pad = deepcopy(bell), deepcopy(pad)
    menu_bell.update(volume=60, reverb=28)
    menu_pad["volume"] = 30
    menu_lead, menu_arp, menu_bass, menu_harmony = [], [], [], []
    for b in range(16):
        phrase = melody_a if b < 8 else melody_b
        menu_lead.append([note(48+p, t, min(d+1, 16-t), 70, 28)
                          for t, p, d in phrase[b % 8] if t in (0, 6, 7, 8, 12)])
        menu_arp.append([note(chords[b % 8][i]+12, i*4+2, 2, 45, 15) for i in range(4)])
        menu_bass.append([note(roots[b % 8], 0, 12, 55, 20)])
        menu_harmony.append([note(chords[b % 8], 0, 15, 50, 30)])
    save(SOURCE / "beepbox/snowbound-menu.json",
         song(84, 16, [channel(menu_bell, menu_lead, 4), channel(arp, menu_arp, 3),
                       channel(bass, menu_bass, 1), channel(menu_pad, menu_harmony, 2)]))

    jingle_bell = deepcopy(bell)
    jingle_bell.update(volume=78, reverb=14, fadeOutTicks=12)
    jingle_bass = deepcopy(bass)
    jingle_bass["volume"] = 55
    jingles = {
        "start": song(240, 1, [channel(jingle_bell, [[note(48+p, t, d, 90, 50)
            for t, p, d in [(0,0,2),(2,7,2),(4,10,2),(6,14,3),(10,19,5)]]], 4)]),
        "level-clear": song(180, 2, [channel(jingle_bell, [
            [note(48+p,t,d,95,55) for t,p,d in [(0,0,2),(2,3,2),(4,7,2),(6,10,2),(8,14,3),(12,19,3)]],
            [note([55,58,62,67],0,13,88,25)]],4),
            channel(jingle_bass,[[note(24,0,7,80,40),note(31,8,7,65,30)],
                                 [note(24,0,13,75,20)]],1)]),
        "game-over": song(150, 2, [channel(jingle_bell, [
            [note(48+p,t,d,85,40) for t,p,d in [(0,10,3),(4,7,3),(8,3,3),(12,2,3)]],
            [note([48,51,55],0,13,72,10)]],4),
            channel(jingle_bass,[[note(34,0,7,70,25),note(31,8,7,60,20)],
                                 [note(24,0,13,70,0)]],1)]),
    }
    for name, data in jingles.items():
        save(SOURCE / f"beepbox/{name}.json", data)

    # sfxr values are normalized controls; these patches are edited/rendered in jsfxr.
    base = dict(oldParams=True, wave_type=2, p_env_attack=0, p_env_sustain=.06,
                p_env_punch=0, p_env_decay=.12, p_base_freq=.3, p_freq_limit=0,
                p_freq_ramp=0, p_freq_dramp=0, p_vib_strength=0, p_vib_speed=0,
                p_arp_mod=0, p_arp_speed=0, p_duty=0, p_duty_ramp=0,
                p_repeat_speed=0, p_pha_offset=0, p_pha_ramp=0,
                p_lpf_freq=1, p_lpf_ramp=0, p_lpf_resonance=0,
                p_hpf_freq=0, p_hpf_ramp=0, sound_vol=.22,
                sample_rate=44100, sample_size=16)
    patches = {
        "wall-tick": dict(wave_type=2,p_env_sustain=.035,p_env_decay=.09,p_base_freq=.34,p_freq_ramp=-.12),
        "paddle-knock": dict(wave_type=0,p_env_sustain=.04,p_env_decay=.14,p_base_freq=.21,
                             p_freq_ramp=-.22,p_duty=.35,p_lpf_freq=.38,p_env_punch=.15),
        "frost-shatter": dict(wave_type=3,p_env_attack=.002,p_env_sustain=.03,p_env_decay=.18,
                              p_base_freq=.57,p_freq_ramp=-.22,p_hpf_freq=.07,p_lpf_freq=.48,
                              p_lpf_ramp=-.15,p_lpf_resonance=.22,p_env_punch=.4,sound_vol=.17),
        "frost-shatter-ring": dict(wave_type=2,p_env_attack=.006,p_env_sustain=.015,p_env_decay=.24,
                                   p_base_freq=.66,p_freq_ramp=-.08,p_arp_mod=-.22,p_arp_speed=.78,
                                   p_hpf_freq=.03,p_lpf_freq=.72,sound_vol=.12),
        "rock-hit": dict(wave_type=3,p_env_sustain=.035,p_env_decay=.16,p_base_freq=.17,
                         p_freq_ramp=-.25,p_lpf_freq=.3,p_lpf_ramp=-.08,p_env_punch=.28),
        "rock-break": dict(wave_type=3,p_env_sustain=.12,p_env_decay=.29,p_base_freq=.14,
                           p_freq_ramp=-.28,p_lpf_freq=.38,p_lpf_ramp=-.14,
                           p_pha_offset=-.13,p_pha_ramp=-.08,p_env_punch=.35,sound_vol=.21),
        "lava-ignite": dict(wave_type=3,p_env_attack=.025,p_env_sustain=.21,p_env_decay=.38,
                            p_base_freq=.13,p_freq_ramp=.15,p_lpf_freq=.25,
                            p_lpf_ramp=.2,p_lpf_resonance=.22,p_repeat_speed=.48,
                            p_pha_offset=.16,p_pha_ramp=-.12,p_env_punch=.18,sound_vol=.18),
        "life-lost": dict(wave_type=2,p_env_sustain=.12,p_env_decay=.29,p_base_freq=.39,
                          p_freq_ramp=-.28,p_freq_dramp=-.08,p_arp_mod=-.25,p_arp_speed=.65),
        "menu-back": dict(wave_type=2,p_env_sustain=.06,p_env_decay=.17,p_base_freq=.43,
                          p_arp_mod=-.26,p_arp_speed=.8,sound_vol=.18),
        "pause": dict(wave_type=2,p_env_sustain=.065,p_env_decay=.14,p_base_freq=.36,
                      p_arp_mod=-.32,p_arp_speed=.83,sound_vol=.18),
        "resume": dict(wave_type=2,p_env_sustain=.065,p_env_decay=.14,p_base_freq=.3,
                       p_arp_mod=.35,p_arp_speed=.83,sound_vol=.18),
        "launch": dict(wave_type=2,p_env_sustain=.04,p_env_decay=.13,p_base_freq=.27,
                       p_freq_ramp=.16,p_arp_mod=.2,p_arp_speed=.86,sound_vol=.17),
    }
    for name, changes in patches.items():
        patch = {**base, **changes}
        save(SOURCE / f"jsfxr/{name}.json", patch)
    print("Authored 5 BeepBox scores and 12 jsfxr patches. Render/export them in the browser.")


if __name__ == "__main__":
    compose()
