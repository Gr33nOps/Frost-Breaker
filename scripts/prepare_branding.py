import base64
import io
import json
import subprocess
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
MEDIA = ROOT / 'docs/media'
SOURCE = ROOT / 'assets/branding'


def piskel(name, layers):
    output = []
    for title, image in layers:
        stream = io.BytesIO()
        image.save(stream, format='PNG')
        output.append(json.dumps(dict(name=title, opacity=1, frameCount=1, chunks=[dict(
            layout=[[0]], base64PNG='data:image/png;base64,' + base64.b64encode(stream.getvalue()).decode())])))
    project = dict(modelVersion=2, piskel=dict(name=name, description='Frost Breaker pixel-art branding',
                   fps=12, width=layers[0][1].width, height=layers[0][1].height,
                   layers=output, hiddenFrames=['']))
    (SOURCE / f'{name}.piskel').write_text(json.dumps(project), encoding='utf-8')


def main():
    MEDIA.mkdir(parents=True, exist_ok=True)
    SOURCE.mkdir(parents=True, exist_ok=True)
    images = ROOT / 'assets/manual-redraw/png'
    stone = Image.new('RGBA', (64, 64))
    ink = ImageDraw.Draw(stone)
    ink.polygon([(4,0),(59,0),(63,4),(63,59),(59,63),(4,63),(0,59),(0,4)], fill='#284955')
    ink.polygon([(5,2),(58,2),(61,5),(61,58),(58,61),(5,61),(2,58),(2,5)], fill='#416777')
    ink.polygon([(2,5),(5,2),(58,2),(61,5),(56,4),(7,4),(4,7),(4,54),(2,58)], fill='#7fa4af')
    ink.polygon([(4,58),(9,50),(20,48),(17,60),(5,61)], fill='#365b6c')
    ink.polygon([(51,5),(60,8),(60,28),(55,23)], fill='#52788a')
    for x,y in [(9,26),(53,36),(8,41),(48,46),(23,7),(53,55)]:
        ink.rectangle((x,y,x+1,y),fill='#668b98')
    frost = Image.new('RGBA', (64, 64))
    brick = Image.open(images / 'Brick.png').convert('RGBA')
    for position in [(12,10),(33,10),(22,21)]:
        frost.alpha_composite(brick, position)
    player = Image.new('RGBA', (64, 64))
    player.alpha_composite(Image.open(images / 'Ball.png').convert('RGBA'),(27,34))
    player.alpha_composite(Image.open(images / 'Paddle.png').convert('RGBA'),(12,48))
    layers = [('Chipped slate badge',stone),('Frost bricks',frost),('Stone ball and paddle',player)]
    icon = Image.new('RGBA', (64, 64))
    for _,layer in layers:icon.alpha_composite(layer)
    icon.save(SOURCE / 'icon-native.png')
    icon.resize((256,256),Image.Resampling.NEAREST).save(MEDIA / 'icon.png')
    icon.resize((256,256),Image.Resampling.NEAREST).save(MEDIA / 'frost-breaker.ico',
        sizes=[(16,16),(32,32),(48,48),(64,64),(128,128),(256,256)])
    piskel('icon',layers)
    logo = Image.open(images / 'logo.png').convert('RGBA')
    logo.resize((580,104),Image.Resampling.NEAREST).save(MEDIA / 'logo.png')
    for suffix in ['piskel','aseprite']:
        source = ROOT / f'assets/manual-redraw/{suffix}/logo.{suffix}'
        (SOURCE / f'logo.{suffix}').write_bytes(source.read_bytes())
    background = Image.open(images / 'menu background.png').convert('RGBA').crop((0,0,315,250))
    wordmark = Image.new('RGBA', (315,250));wordmark.alpha_composite(logo,(12,34))
    emblem = Image.new('RGBA', (315,250));emblem.alpha_composite(icon,(125,118))
    cover = background.copy();cover.alpha_composite(wordmark);cover.alpha_composite(emblem)
    cover.resize((630,500),Image.Resampling.NEAREST).save(MEDIA / 'itch-cover.png')
    piskel('itch-cover',[('Frost valley',background),('Frosted wordmark',wordmark),('Slate game emblem',emblem)])
    aseprite = Path('D:/aseprite/build/bin/aseprite.exe')
    if aseprite.exists():
        work = ROOT / 'build/branding'
        work.mkdir(parents=True, exist_ok=True)
        lua = ['local s = Sprite(64,64,ColorMode.RGB)']
        for index,(name,image) in enumerate(layers):
            path = work / f'layer-{index}.png'
            image.save(path)
            lua += [f'local layer = {"s.layers[1]" if index == 0 else "s:newLayer()"}; layer.name = {json.dumps(name)}',
                    f's:newCel(layer,1,Image{{fromFile={json.dumps(path.as_posix())}}},Point(0,0))']
        lua += [f's:saveAs({json.dumps((SOURCE / "icon.aseprite").as_posix())})']
        script = work / 'icon.lua'
        script.write_text('\n'.join(lua))
        subprocess.run([str(aseprite),'-b','--script',str(script)],check=True,capture_output=True)
    print('Saved pixel-art wordmark, three-layer Piskel icon, editable Aseprite icon, and itch cover')


if __name__ == '__main__':
    main()
