"""Combine actual game captures into a native-scale overview of all ten levels."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]


def main():
    names = (ROOT / "assets/design/campaign-names.txt").read_text(encoding="utf-8").splitlines()
    if len(names) != 10:
        raise ValueError("Expected ten chapter names from PreviewArt.exe")
    font = ImageFont.truetype(str(ROOT / "assets/fonts/Jersey10-Regular.ttf"), 24)
    sheet = Image.new("RGB", (1372, 424), "#284955")
    draw = ImageDraw.Draw(sheet)
    for number, name in enumerate(names, 1):
        x = 12 + ((number - 1) % 5) * 272
        y = 12 + ((number - 1) // 5) * 206
        draw.text((x + 4, y), f"{number:02}  {name}", font=font, fill="#edf7f7")
        with Image.open(ROOT / f"assets/design/level-{number:02}-preview.png") as image:
            thumbnail = image.crop((0, 36, 520, 376)).resize((260, 170), Image.Resampling.NEAREST)
            sheet.paste(thumbnail, (x, y + 28))
    sheet.save(ROOT / "assets/design/campaign-preview.png")
    print("Rendered all ten campaign layouts into assets/design/campaign-preview.png")


if __name__ == "__main__":
    main()
