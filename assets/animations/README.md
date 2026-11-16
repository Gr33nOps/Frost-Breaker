# Pixel animations

Each clip has an editable Aseprite source, a native horizontal sheet, a 2x runtime sheet, and timing in `manifest.json`.

| Clip | Native frame | Frames | Frame time |
| --- | --- | --- | --- |
| BallGlint | 10 x 10 | 6 | 120 ms |
| LavaBallFlicker | 10 x 10 | 4 | 100 ms |
| LavaPickupPulse | 19 x 9 | 4 | 140 ms |
| IceBurst | 24 x 24 | 6 | 60 ms |
| RockChip | 24 x 24 | 6 | 60 ms |
| LavaBurst | 24 x 24 | 6 | 60 ms |

The ball and pickup clips loop. Impact clips play once at the contact point; stone impacts use half-size chips. The lava trail uses fading pixel squares in the game renderer. Animations pause with gameplay.

Edit `aseprite/` sources, then use `python scripts/export_pixel_animations.py` to export them. Verify timing, frames and pixels with `python tests/verify_pixel_animations.py`.
