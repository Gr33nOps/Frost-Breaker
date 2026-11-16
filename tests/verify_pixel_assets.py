"""Decode the delivered Aseprite cels and compare with the game PNG exports."""
from pathlib import Path
import json
import struct
import zlib
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[1]
manifest = json.loads((ROOT/'assets/aseprite/manifest.json').read_text())


def read_aseprite(path):
    data = path.read_bytes()
    length, magic, frames, width, height, depth = struct.unpack_from('<I5H', data)
    assert length == len(data) and magic == 0xA5E0 and frames == 1 and depth == 32, path
    assert data[34:36] == b'\x01\x01', 'non-square source pixels'
    frame_size, frame_magic, count = struct.unpack_from('<IHH', data, 128)
    assert frame_magic == 0xF1FA and frame_size == len(data)-128, path
    offset = 144
    layers = []
    cels = {}
    for _ in range(count):
        chunk_size, chunk_type = struct.unpack_from('<IH', data, offset)
        assert chunk_size >= 6 and offset+chunk_size <= len(data), path
        payload = data[offset+6:offset+chunk_size]
        if chunk_type == 0x2004:
            flags, kind, child = struct.unpack_from('<3H', payload)
            assert flags & 1 and kind == 0 and child == 0
            name_length = struct.unpack_from('<H', payload, 16)[0]
            layers.append((payload[18:18+name_length].decode('utf-8'), payload[12]))
        elif chunk_type == 0x2005:
            index, x, y, opacity, kind = struct.unpack_from('<HhhBH', payload)
            assert kind == 2
            cel_width, cel_height = struct.unpack_from('<HH', payload, 16)
            pixels = zlib.decompress(payload[20:])
            assert len(pixels) == cel_width*cel_height*4
            image = Image.frombytes('RGBA', (cel_width,cel_height), pixels)
            image.putalpha(image.getchannel('A').point(lambda v: round(v*opacity/255)))
            canvas = Image.new('RGBA',(width,height))
            canvas.paste(image,(x,y))
            cels[index] = canvas
        offset += chunk_size
    assert offset == len(data) and len(layers) == len(cels)
    result = Image.new('RGBA',(width,height))
    for index in range(len(layers)):
        image = cels[index]
        image.putalpha(image.getchannel('A').point(lambda v: round(v*layers[index][1]/255)))
        result = Image.alpha_composite(result,image)
    return result, [layer[0] for layer in layers]


for asset in manifest['assets']:
    native, layers = read_aseprite(ROOT/asset['source'])
    assert list(native.size) == asset['canvas'] and layers == asset['layers'], asset['name']
    export = Image.open(ROOT/asset['png']).convert('RGBA')
    expected = native.resize((native.width*2,native.height*2),Image.Resampling.NEAREST)
    assert export.size == expected.size
    assert ImageChops.difference(export,expected).getbbox(alpha_only=False) is None, asset['name']
    print('PASS editable source and exact 2x export:',asset['name'])

assert Image.open(ROOT/'assets/images/Ball.png').size == (20,20)
assert Image.open(ROOT/'assets/images/LavaBall.png').size == (20,20)
assert Image.open(ROOT/'assets/images/Paddle.png').size == (80,22)
assert Image.open(ROOT/'assets/images/LavaPickup.png').size == (38,18)
for name in ('Brick','Rock','RockCracked'):
    assert Image.open(ROOT/f'assets/images/{name}.png').size == (38,18)
print(f"{len(manifest['assets'])} Aseprite files and PNG exports verified; runtime dimensions match.")
