"""Export native Aseprite animation clips as untrimmed 2x horizontal sheets."""
from pathlib import Path
import argparse
import json
import subprocess
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "assets/animations"
ASEPRITE = "D:/aseprite/build/bin/aseprite.exe"
CLIPS = [("BallGlint", 10, 10, 6, 120, True),
         ("LavaBallFlicker", 10, 10, 4, 100, True),
         ("LavaPickupPulse", 19, 9, 4, 140, True),
         ("IceBurst", 24, 24, 6, 60, False),
         ("RockChip", 24, 24, 6, 60, False),
         ("LavaBurst", 24, 24, 6, 60, False)]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--redraw", action="store_true", help="Reconstruct all native frames, replacing Aseprite edits")
    args = parser.parse_args()
    for folder in ["aseprite", "png", "native"]:
        (OUT / folder).mkdir(parents=True, exist_ok=True)
    if args.redraw or any(not (OUT / "aseprite" / (clip[0] + ".aseprite")).exists() for clip in CLIPS):
        result = subprocess.run([ASEPRITE, "-b", "--script-param", "root=" + ROOT.as_posix(),
            "--script", str(ROOT / "scripts/draw_pixel_animations.lua")], capture_output=True, text=True)
        if result.returncode:
            raise RuntimeError(result.stdout + result.stderr)
    manifest = []
    for name, w, h, count, duration, loop in CLIPS:
        source = OUT / "aseprite" / (name + ".aseprite")
        for folder, scale in [("native", 1), ("png", 2)]:
            subprocess.run([ASEPRITE, "-b", str(source), "--scale", str(scale),
                "--sheet-type", "horizontal", "--sheet", str(OUT / folder / (name + ".png"))],
                check=True, capture_output=True)
            assert Image.open(OUT / folder / (name + ".png")).size == (w * count * scale, h * scale)
        manifest.append(dict(name=name, canvas=[w,h], frames=count, durationMs=duration,
            loop=loop, source=f"assets/animations/aseprite/{name}.aseprite",
            png=f"assets/animations/png/{name}.png"))
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2))

    # Contact sheet plays the exact exported frames without smoothing.
    previews = []
    for tick in range(36):
        image = Image.new("RGB", (720, 300), "#315969")
        draw = ImageDraw.Draw(image)
        for i, (name, w, h, count, duration, loop) in enumerate(CLIPS):
            sheet = Image.open(OUT / "native" / (name + ".png")).convert("RGBA")
            phase = (tick * 50) if loop else ((tick * 50) % 900)
            frame = phase // duration
            x, y = 120 + i % 3 * 240, 65 + i // 3 * 150
            draw.text((x-70, y+62), name, fill="#dcffff")
            if loop or frame < count:
                frame %= count
                sprite = sheet.crop((frame*w, 0, (frame+1)*w, h)).resize((w*5,h*5), Image.Resampling.NEAREST)
                image.paste(sprite, (x-sprite.width//2,y-sprite.height//2), sprite)
        previews.append(image)
    previews[0].save(ROOT / "assets/design/pixel-animations-preview.gif", save_all=True,
        append_images=previews[1:], duration=50, loop=0, disposal=2)
    print("Exported 6 editable animation clips and exact 2x sprite sheets.")


if __name__ == "__main__":
    main()
