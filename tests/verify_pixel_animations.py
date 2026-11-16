"""Independently decode animated Aseprite frames and compare both sheet scales."""
from pathlib import Path
import json
import struct
import zlib
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[1]


def read_frames(path):
    data = path.read_bytes()
    length, magic, count, width, height, depth = struct.unpack_from("<I5H", data)
    assert length == len(data) and magic == 0xA5E0 and depth == 32
    layers, records, durations = [], [], []
    offset = 128
    for _ in range(count):
        frame_size, frame_magic, chunks, duration = struct.unpack_from("<IHHH", data, offset)
        assert frame_magic == 0xF1FA
        if chunks == 65535:
            chunks = struct.unpack_from("<I", data, offset + 12)[0]
        end = offset + frame_size
        cursor = offset + 16
        cels = {}
        for _ in range(chunks):
            size, kind = struct.unpack_from("<IH", data, cursor)
            payload = data[cursor + 6:cursor + size]
            if kind == 0x2004:
                flags, layer_kind = struct.unpack_from("<2H", payload)
                assert flags & 1 and layer_kind == 0
                layers.append(payload[12])
            elif kind == 0x2005:
                layer, x, y, opacity, encoding = struct.unpack_from("<HhhBH", payload)
                if encoding == 1:
                    content = struct.unpack_from("<H", payload, 16)[0]
                else:
                    assert encoding in (0, 2)
                    w, h = struct.unpack_from("<HH", payload, 16)
                    pixels = zlib.decompress(payload[20:]) if encoding == 2 else payload[20:]
                    assert len(pixels) == w*h*4
                    content = Image.frombytes("RGBA", (w,h), pixels)
                cels[layer] = (x,y,opacity,encoding,content)
            cursor += size
        assert cursor == end
        records.append(cels)
        durations.append(duration)
        offset = end
    assert offset == len(data)

    def image_for(frame, layer):
        cel = records[frame][layer]
        return image_for(cel[4],layer) if cel[3] == 1 else cel[4]

    frames = []
    for frame, cels in enumerate(records):
        output = Image.new("RGBA", (width,height))
        for index, layer_opacity in enumerate(layers):
            if index not in cels:
                continue
            x,y,opacity,_,_ = cels[index]
            image = image_for(frame,index).copy()
            image.putalpha(image.getchannel("A").point(lambda a: round(a*opacity*layer_opacity/65025)))
            canvas = Image.new("RGBA", output.size)
            canvas.paste(image,(x,y))
            output = Image.alpha_composite(output,canvas)
        frames.append(output)
    return frames, durations


def main():
    manifest = json.loads((ROOT / "assets/animations/manifest.json").read_text())
    total = 0
    for clip in manifest:
        frames, durations = read_frames(ROOT / clip["source"])
        w,h = clip["canvas"]
        assert len(frames) == clip["frames"] and all(d == clip["durationMs"] for d in durations)
        assert len({frame.tobytes() for frame in frames}) > 1, clip["name"]
        native = Image.open(ROOT / "assets/animations/native" / (clip["name"] + ".png")).convert("RGBA")
        game = Image.open(ROOT / clip["png"]).convert("RGBA")
        assert native.size == (w*len(frames),h) and game.size == (w*len(frames)*2,h*2)
        for i, frame in enumerate(frames):
            assert frame.size == (w,h)
            assert ImageChops.difference(frame,native.crop((i*w,0,(i+1)*w,h))).getbbox(alpha_only=False) is None
            scaled = frame.resize((w*2,h*2),Image.Resampling.NEAREST)
            assert ImageChops.difference(scaled,game.crop((i*w*2,0,(i+1)*w*2,h*2))).getbbox(alpha_only=False) is None
        if clip["name"] in ("BallGlint","LavaBallFlicker","LavaPickupPulse"):
            static = {"BallGlint":"Ball","LavaBallFlicker":"LavaBall","LavaPickupPulse":"LavaPickup"}[clip["name"]]
            mask = Image.open(ROOT / "assets/manual-redraw/png" / (static + ".png")).getchannel("A")
            assert all(frame.getchannel("A").tobytes() == mask.tobytes() for frame in frames)
            if clip["name"] == "LavaPickupPulse":
                reference=Image.open(ROOT / "assets/manual-redraw/png/LavaPickup.png").convert("RGBA")
                assert ImageChops.difference(frames[0],reference).getbbox(alpha_only=False) is None
        total += len(frames)
        print("PASS editable animation, timing and exact pixel exports:", clip["name"])
    print(f"Verified {len(manifest)} animated Aseprite files and {total} frames.")


if __name__ == "__main__":
    main()
