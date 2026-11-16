# Pixel-art working files

The artwork uses frost cyan, slate blue and warm lava accents. Draw at native resolution with a one-pixel pencil; game images use exactly 2x nearest-neighbor scaling.

| Asset | Native canvas |
| --- | --- |
| Ball | 10 x 10 |
| Paddle | 40 x 11 |
| Frost, rock and lava brick | 19 x 9 |
| Gameplay/menu background | 400 x 300 |
| Logo | 290 x 52 |
| Normal/hover button | 112 x 34 |
| Gameplay overlay | 260 x 300 |
| Heart | 15 x 13 |

`piskel/` contains editable browser-editor projects. `aseprite/` contains their native layered conversions, `png/` their native exports, and `game-png/` the 2x exports. Two extra source canvases hold the button base and sidebar texture.

The installed game sources are in `assets/aseprite/`, animations in `assets/animations/`, and release branding in `assets/branding/`.

Edit the Aseprite source and export to `assets/images/`, or edit a Piskel project and run `python scripts/export_manual_art.py --install`. The exporter reads Piskel projects, so it will overwrite changes made only to an Aseprite copy. Its default Aseprite executable path can be changed with `--aseprite`.

Verify exports with `python tests/verify_pixel_assets.py`. Original one-off drawing recipes and dated backups stay in the local `build/archive/` folder and are excluded from the repository.
