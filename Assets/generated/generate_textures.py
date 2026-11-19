
from pathlib import Path
import random
from PIL import Image,ImageDraw
dest=Path(__file__).resolve().parent
r=random.Random(2415)
for name,base in [('grass',(74,110,48)),('road',(131,126,110)),('wall',(118,116,108))]:
 im=Image.new('RGB',(128,128));p=im.load()
 for y in range(128):
  for x in range(128):
   n=r.randint(-14,14);p[x,y]=tuple(max(0,min(255,c+n)) for c in base)
 d=ImageDraw.Draw(im)
 if name!='grass':
  for y in range(0,128,32):
   d.line((0,y,127,y),fill=(65,64,59),width=2)
   for x in range(-16 if (y//32)%2 else 0,128,32):d.line((x,y,x,y+32),fill=(65,64,59),width=2)
 im.save(dest/(name+'.png'))
