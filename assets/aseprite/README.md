# Frost Breaker pixel-art sources

These are editable, layered RGBA Aseprite files. Draw at the native canvas resolution with a 1 px pencil. The game PNGs are exactly 2x, without smoothing.

| Source | Native canvas | Game export |
|---|---:|---:|
| background.aseprite | 400 x 300 | 800 x 600 |
| menu background.aseprite | 400 x 300 | 800 x 600 |
| logo.aseprite | 290 x 52 | 580 x 104 |
| logo2.aseprite | 290 x 52 | 580 x 104 |
| start.aseprite | 112 x 34 | 224 x 68 |
| start2.aseprite | 112 x 34 | 224 x 68 |
| Quit.aseprite | 112 x 34 | 224 x 68 |
| quit2.aseprite | 112 x 34 | 224 x 68 |
| resume.aseprite | 112 x 34 | 224 x 68 |
| resume2.aseprite | 112 x 34 | 224 x 68 |
| play again.aseprite | 112 x 34 | 224 x 68 |
| play again2.aseprite | 112 x 34 | 224 x 68 |
| menu.aseprite | 112 x 34 | 224 x 68 |
| menu2.aseprite | 112 x 34 | 224 x 68 |
| try again.aseprite | 112 x 34 | 224 x 68 |
| overlay.aseprite | 260 x 300 | 520 x 600 |
| overlay2.aseprite | 400 x 300 | 800 x 600 |
| Ball.aseprite | 10 x 10 | 20 x 20 |
| LavaBall.aseprite | 10 x 10 | 20 x 20 |
| Paddle.aseprite | 40 x 11 | 80 x 22 |
| Brick.aseprite | 19 x 9 | 38 x 18 |
| Rock.aseprite | 19 x 9 | 38 x 18 |
| RockCracked.aseprite | 19 x 9 | 38 x 18 |
| RockEmpty.aseprite | 19 x 9 | 38 x 18 |
| RockLava.aseprite | 19 x 9 | 38 x 18 |
| heart.aseprite | 15 x 13 | 30 x 26 |
| 1 lives.aseprite | 110 x 13 | 220 x 26 |
| 2 lives.aseprite | 110 x 13 | 220 x 26 |
| 3 lives.aseprite | 110 x 13 | 220 x 26 |
| 4 lives.aseprite | 110 x 13 | 220 x 26 |
| 5 lives.aseprite | 110 x 13 | 220 x 26 |
| PixelFont.aseprite | 264 x 8 | 528 x 16 |

Open a source in Aseprite and draw with a one-pixel pencil. Export at 200% with nearest-neighbor scaling to its matching `assets/images/` filename. Keep the editable source at native resolution.

The sidebar, buttons, normal ball and chipped paddle share a slate-blue stone palette. The logo blends frost cyan into stone. Separate layers hold the surface grain, facets, lettering and outlines. The bright landscape and transparent gameplay overlay keep their own layers.

Rocks are permanent obstacles. Levels contain one to three separate lava power bricks. Every collected lava brick adds ten seconds to the remaining power time; losing a life or changing levels clears it. Legacy cracked-rock artwork is retained as an editable drawing but is not used for rock damage.

The optional PixelFont atlas has 6 x 8 native cells, with its glyph order in `manifest.json`. Live game text uses Jersey 10 from `assets/fonts/`.

`assets/manual-redraw/piskel/` holds the matching Piskel working files. `scripts/export_manual_art.py` converts those saved drawings and checks Aseprite PNG round trips. Export directly from Aseprite to preserve edits made only to an Aseprite source.
