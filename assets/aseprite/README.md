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

Open any source directly in Aseprite. Export PNG at 200% with nearest-neighbor to the matching assets/images filename. Keep the source at native size. Sprites and scenery began as Piskel pixel drawings. The revised panel, ball and paddle use the rock's blue-grey palette; buttons use frost cyan: buttons have carved frost edges and separate text, outline and shadow layers; the logo has separate faces, bevel, extrusion, frost edges and readable e/a/s openings. Mountain streaks were replaced with broad muted faces using Aseprite's pixel API. Lives rows have a layer for each heart. The gameplay background separates its playfield, sidebar body and frame; internal decorations are removed. The normal ball and paddle use rock colours; the lava ball retains its molten palette. The paddle has a stone-slab silhouette, chipped underside, broad facets and small surface seams on three editable layers. Its 40 x 11 canvas and collision size are preserved.

Rocks take three hits. Hit one uses RockCracked; hit two reveals RockEmpty or RockLava. Hit three destroys the rock and grants lava only if it contained lava. All frost and rock states share the same canvas and collision dimensions. Crack marks are separate editable layers. There is no standalone lava block. The optional PixelFont atlas is not used by the restored HUD. Its glyph order is recorded in manifest.json; preserve the 6 x 8 native cells when editing it.

The current sources and downloadable archive are in assets/manual-redraw/. scripts/export_manual_art.py converts the saved drawings and verifies Aseprite PNG round trips. The older scripts/draw_pixel_assets.py recreates the rejected artwork and must not be used for this set. Use Aseprite export to preserve subsequent edits. The try again2.aseprite hover variant also uses a 112 x 34 canvas.
