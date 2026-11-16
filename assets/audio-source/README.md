# Original music and sound

Music was composed in [BeepBox](https://www.beepbox.co/) and effects in [jsfxr](https://sfxr.me/). No sampled songs or third-party sound packs are used.

- `beepbox/`: five editable JSON scores and song URLs.
- `jsfxr/`: twelve saved parameter patches, editor permalinks and export metadata.
- `manifest.json`: runtime paths, tempo, duration and source mapping.
- `assets/sounds/`: the mastered OGG music and WAV effects used by the game.

**Snowbound** plays in the menu at 84 BPM; **Glacier Run** accompanies gameplay at 112 BPM. Start, level-clear and game-over jingles are also editable BeepBox scores.

Open a song URL, or import its JSON in BeepBox. Export music as WAV with two full-song loops, so the mastering script can use the second loop's settled effect tails. Export jingles once. Save WAV files in `browser-exports/` with the names in the manifest.

For effects, open the permalink or deserialize the JSON in jsfxr, edit the controls, and export 44.1 kHz / 16-bit WAV. Save the updated patch and matching WAV in `browser-exports/`.

Run `python scripts/prepare_audio.py` to balance levels, remove DC, soften edges, extract loops and encode OGG. It requires NumPy and ffmpeg. `--sfx frost-shatter` rebuilds only the frost effect: a filtered crack with a quiet crystal tail. Then run `python tests/verify_audio.py`.

Raw browser exports, WAV masters and listening previews are kept locally and excluded from Git to avoid duplicate large audio files. Export them from the saved editor projects before remastering. Existing runtime audio needs no rebuilding to play.

`compose_audio_projects.py` records the original notes and instrument settings. It replaces saved project edits when run; use the browser editors for normal revisions.
