from pathlib import Path
import json,shutil
from reportlab.pdfgen import canvas
from reportlab.lib.utils import ImageReader
from pypdf import PdfReader
from PIL import Image
R=Path(__file__).resolve().parents[2]
T=R/'tmp/presentation/local';O=R/'output/presentation'
links=json.loads((T/'links.json' if (T/'links.json').exists() else O/'links.json').read_text())
out=O/'Monster_Clash_Presentation.pdf'
c=canvas.Canvas(str(out),pagesize=(960,540))
c.setTitle('Monster Clash — Final Project Presentation')
c.setAuthor('Aibyn Samat, Diyar Sekezhanov, Dias Zhakin')
for i in range(1,11):
    c.drawImage(ImageReader(str(T/f'final-{i:02}.png')),0,0,width=960,height=540)
    for link in links:
        if link['slide']==i:
            x,y,w,h=[link[k]*0.75 for k in ['x','y','w','h']]
            c.linkURL(link['href'],(x,540-y-h,x+w,540-y),relative=0,thickness=0)
    c.showPage()
c.save()
shutil.copy2(out,R/'output/pdf/Monster_Clash_Presentation.pdf')
report=R/'output/pdf/Monster_Clash_Project_Report.pdf'
assert len(PdfReader(out).pages)==10
assert len(PdfReader(report).pages)==13
annotations=[a.get_object().get('/A',{}).get('/URI') for page in PdfReader(out).pages for a in page.get('/Annots',[])]
assert 'https://youtu.be/-plQpir5YrU' in annotations
im=Image.open(R/'output/pdf/build_capture.png').convert('RGB')
assert any(r!=g or g!=b for r,g,b in im.getdata()),'Report screenshot must retain colour'
print('10-slide linked PDF; 13-page report with colour capture; links verified.')
tiles=[]
for i in range(1,11):
    im=Image.open(T/f'final-{i:02}.png').convert('RGB');im.thumbnail((480,270));tiles.append(im.copy())
for start in range(0,10,4):
    sheet=Image.new('RGB',(960,540),'#ddd')
    for k,im in enumerate(tiles[start:start+4]):sheet.paste(im,((k%2)*480,(k//2)*270))
    sheet.save(T/f'final-contact-{start//4+1}.png')
print(out)
