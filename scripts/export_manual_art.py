"""Export saved Frost Breaker pixel artwork. This script does not draw it.

It decodes saved layers, assembles repeated hearts/background regions, preserves
layer opacity, and uses Aseprite to save layered sources and PNG exports.
"""
from pathlib import Path
from io import BytesIO
import argparse
import base64
import json
import shutil
import struct
import subprocess
import zipfile
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "assets/manual-redraw"
WORK = ROOT / "build/manual-art"
NAMES = ["Ball", "Brick", "Paddle", "heart", "menu background",
         "sidebar landscape", "logo", "button base", "start", "start2",
         "Quit", "quit2", "resume", "resume2", "menu", "menu2",
         "play again", "play again2", "try again", "try again2",
         "overlay", "overlay2", "Rock", "RockCracked", "LavaPickup", "LavaBall"]


def encode(image):
    stream = BytesIO()
    image.save(stream, format="PNG")
    return "data:image/png;base64," + base64.b64encode(stream.getvalue()).decode()


def write_project(name, size, layers):
    records = []
    for title, image, opacity in layers:
        records.append(json.dumps(dict(name=title, opacity=opacity, frameCount=1,
            chunks=[dict(layout=[[0]], base64PNG=encode(image))])))
    payload = dict(modelVersion=2, piskel=dict(name=name, description="Frost Breaker native pixel-art source",
        fps=12, width=size[0], height=size[1], layers=records, hiddenFrames=[""]))
    (OUT / "piskel" / (name + ".piskel")).write_text(json.dumps(payload), encoding="utf-8")


def composite(size, layers):
    result = Image.new("RGBA", size)
    for _, image, opacity in layers:
        image = image.copy()
        image.putalpha(image.getchannel("A").point(lambda v: round(v * opacity)))
        result = Image.alpha_composite(result, image)
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--downloads", type=Path, help="Explicitly import matching downloaded Piskel projects")
    parser.add_argument("--aseprite", default="D:/aseprite/build/bin/aseprite.exe")
    parser.add_argument("--install", action="store_true", help="Copy verified art into the game")
    args = parser.parse_args()
    for folder in [OUT / "png", OUT / "game-png", OUT / "piskel", OUT / "aseprite", WORK]:
        folder.mkdir(parents=True, exist_ok=True)
    projects = {}
    for name in NAMES:
        target = OUT / "piskel" / (name + ".piskel")
        # Match exact names: 'start-' must not match 'start2-'.
        candidates = list(args.downloads.glob(name + "-*.piskel")) if args.downloads else []
        if candidates:
            shutil.copy2(max(candidates, key=lambda p: p.stat().st_mtime), target)
        p = json.loads(target.read_text(encoding="utf-8"))["piskel"]
        size = (p["width"], p["height"])
        layers = []
        for record in p["layers"]:
            layer = json.loads(record)
            assert layer["frameCount"] == 1
            assert len(layer["chunks"]) == 1 and layer["chunks"][0]["layout"] == [[0]]
            image = Image.open(BytesIO(base64.b64decode(
                layer["chunks"][0]["base64PNG"].split(",", 1)[1]))).convert("RGBA")
            assert image.size == size, (name, image.size, size)
            layers.append((layer["name"], image, layer["opacity"]))
        projects[name] = (size, layers)
        write_project(name, size, layers)

    light = composite(*projects["menu background"])
    field = Image.new("RGBA", (400, 300))
    field.paste(light.crop((0, 0, 260, 300)), (0, 0))
    projects["background"] = ((400, 300), [("Snowy playfield", field, 1),
        *[(title, image, opacity) for title, image, opacity in projects["sidebar landscape"][1]]])
    projects["logo2"] = projects["logo"]
    heart = composite(*projects["heart"])
    for count in range(1, 6):
        layers = []
        for i in range(count):
            row = Image.new("RGBA", (110, 13))
            row.paste(heart, (22 * i, 0))
            layers.append((f"Heart {i + 1}", row, 1))
        projects[f"{count} lives"] = ((110, 13), layers)

    lua = []
    manifest = []
    quote = lambda p: json.dumps(str(p).replace("\\", "/"))
    for index, (name, (size, layers)) in enumerate(projects.items()):
        write_project(name, size, layers)
        rendered = composite(size, layers)
        rendered.save(OUT / "png" / (name + ".png"))
        if name not in {"sidebar landscape", "button base"}:
            rendered.resize((size[0] * 2, size[1] * 2), Image.Resampling.NEAREST).save(
                OUT / "game-png" / (name + ".png"))
        lua += ["do", f"local s = Sprite({size[0]}, {size[1]})"]
        for j, (title, image, opacity) in enumerate(layers):
            path = WORK / f"layer-{index}-{j}.png"
            image.save(path)
            lua += ["local l = " + ("s.layers[1]" if j == 0 else "s:newLayer()"),
                    f"l.name = {json.dumps(title)}", f"l.opacity = {round(opacity * 255)}",
                    f"s:newCel(l, 1, Image{{fromFile={quote(path)}}})"]
        ase = OUT / "aseprite" / (name + ".aseprite")
        check = WORK / (name + "-roundtrip.png")
        lua += [f"s:saveAs({quote(ase)})", f"s:saveCopyAs({quote(check)})", "s:close()", "end"]
        manifest.append(dict(name=name, width=size[0], height=size[1], layers=len(layers)))
    script = WORK / "export.lua"
    script.write_text("\n".join(lua), encoding="utf-8")
    subprocess.run([args.aseprite, "-b", "--script", str(script)], check=True)
    for item in manifest:
        name = item["name"]
        ase = OUT / "aseprite" / (name + ".aseprite")
        header = ase.read_bytes()[:128]
        assert struct.unpack_from("<H", header, 4)[0] == 0xA5E0
        assert struct.unpack_from("<HH", header, 8) == (item["width"], item["height"])
        expected = Image.open(OUT / "png" / (name + ".png")).convert("RGBA")
        actual = Image.open(WORK / (name + "-roundtrip.png")).convert("RGBA")
        assert actual.size == expected.size and actual.tobytes() == expected.tobytes(), name
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2))

    sheet = Image.new("RGB", (800, 690), "#132b39")
    draw = ImageDraw.Draw(sheet)
    sheet.paste(light, (0, 0))
    sheet.paste(composite(*projects["background"]), (400, 0))
    draw.text((12, 307), "MENU BACKGROUND                  GAMEPLAY BACKGROUND", fill="white")
    logo = composite(*projects["logo"])
    sheet.paste(logo.resize((580, 104), Image.Resampling.NEAREST), (15, 330),
                logo.resize((580, 104), Image.Resampling.NEAREST))
    for index, name in enumerate(["start", "Quit", "resume", "menu", "play again", "try again"]):
        button = composite(*projects[name]).resize((224, 68), Image.Resampling.NEAREST)
        sheet.paste(button, (15 + index % 3 * 260, 444 + index // 3 * 80), button)
    for index, name in enumerate(["Ball", "LavaBall", "Brick", "Rock", "RockCracked", "LavaPickup", "Paddle", "heart"]):
        image = composite(*projects[name])
        image = image.resize((image.width * 2, image.height * 2), Image.Resampling.NEAREST)
        x = 15 + index * 85
        sheet.paste(image, (x, 625), image)
        draw.text((x, 659), name, fill="white")
    sheet.save(OUT / "preview.png")
    stages = Image.new("RGB", (640, 150), "#0b4553")
    stages_ink = ImageDraw.Draw(stages)
    for index, (name, label) in enumerate([
            ("Rock", "Permanent rock"), ("Brick", "Breakable frost"),
            ("LavaPickup", "Protected lava brick")]):
        source = composite(*projects[name])
        sprite = source.resize((source.width*6,source.height*6), Image.Resampling.NEAREST)
        x = 20 + index * 160
        stages.paste(sprite, (x, 38), sprite)
        stages_ink.text((x, 16), label, fill="#d7f5f2")
    stages_ink.text((20, 110), "Clear frost only. Rocks stay. Five lives for the whole campaign.", fill="#91d3d7")
    stages_ink.text((20, 128), "One lava brick per level. Lava ends on a fall or a new level.", fill="#91d3d7")
    stages.save(ROOT / "assets/design/rock-stages-preview.png")
    if args.install:
        extra = {"sidebar landscape", "button base"}
        game_manifest = []
        for name in projects:
            if name in extra:
                continue
            image = Image.open(OUT / "png" / (name + ".png"))
            image.resize((image.width * 2, image.height * 2), Image.Resampling.NEAREST).save(
                ROOT / "assets/images" / (name + ".png"))
            shutil.copy2(OUT / "aseprite" / (name + ".aseprite"), ROOT / "assets/aseprite" / (name + ".aseprite"))
            size, layers = projects[name]
            game_manifest.append(dict(name=name, canvas=list(size),
                source=f"assets/aseprite/{name}.aseprite", png=f"assets/images/{name}.png",
                layers=[layer[0] for layer in layers]))
        (ROOT / "assets/aseprite/manifest.json").write_text(json.dumps(
            dict(origin="Pixel artwork with matching stone panel, logo, buttons and player sprites, original pixel font and frost bricks", exportScale=2, assets=game_manifest), indent=2))
    archive = OUT / "frost-breaker-manual-pixel-art.zip"
    with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as z:
        for directory in ["aseprite", "piskel", "png", "game-png"]:
            for path in sorted((OUT / directory).iterdir()):
                z.write(path, path.relative_to(OUT))
        for name in ["manifest.json", "preview.png", "README.md"]:
            path = OUT / name
            if path.exists():
                z.write(path, name)
        animation_root = ROOT / "assets/animations"
        if animation_root.exists():
            for path in sorted(animation_root.rglob("*")):
                if path.is_file():
                    z.write(path, "animations/" + path.relative_to(animation_root).as_posix())
        if args.install:
            previews = ["menu-preview.png", "gameplay-preview.png", "ready-preview.png", "pause-preview.png", "win-preview.png", "game-over-preview.png", "lava-preview.png", "rock-stages-preview.png", "stone-paddle-preview.png", "pixel-animations-preview.gif", "animated-gameplay-preview.gif", "campaign-preview.png", "campaign-names.txt"]
            previews.append("logo-theme-preview.png")
            previews += [f"level-{level:02}{suffix}-preview.png" for level in range(1, 11) for suffix in ["", "-ready"]]
            for name in previews:
                path = ROOT / "assets/design" / name
                if path.exists():
                    z.write(path, "previews/" + name)
    with zipfile.ZipFile(archive) as z:
        assert z.testzip() is None, "Archive CRC check failed"
    print(f"Verified {len(projects)} Aseprite files: sizes and PNG round trips match.")
    print(archive)


if __name__ == "__main__":
    main()
