"""Combine actual game captures into a native-scale overview of all ten levels."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]


def main():
    names = (ROOT / "assets/design/campaign-names.txt").read_text(encoding="utf-8").splitlines()
    if len(names) != 10:
        raise ValueError("Expected ten chapter names from PreviewArt.exe")
    font = ImageFont.truetype(str(ROOT / "assets/fonts/Jersey10-Regular.ttf"), 40)
    sheet = Image.new("RGB", (1064, 1974), "#284955")
    draw = ImageDraw.Draw(sheet)
    for number, name in enumerate(names, 1):
        x = 8 + ((number - 1) % 2) * 528
        y = 8 + ((number - 1) // 2) * 392
        draw.text((x + 12, y + 3), f"{number:02}  {name}", font=font, fill="#edf7f7")
        with Image.open(ROOT / f"assets/design/level-{number:02}-preview.png") as image:
            sheet.paste(image.crop((0, 36, 520, 376)), (x, y + 44))
    sheet.save(ROOT / "assets/design/campaign-preview.png")
    print("Rendered all ten campaign layouts into assets/design/campaign-preview.png")


if __name__ == "__main__":
    main()
