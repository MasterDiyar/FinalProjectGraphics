"""Build the required standalone PDF presentation (16:9, monochrome)."""
from pathlib import Path
import csv
from reportlab.pdfgen import canvas
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import Paragraph
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.utils import ImageReader
from PIL import Image

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent.parent
for n,f in [('Body','times.ttf'),('Bold','timesbd.ttf')]:
    pdfmetrics.registerFont(TTFont(n,str(Path('C:/Windows/Fonts')/f)))
OUT=HERE/'Monster_Clash_Presentation.pdf'
c=canvas.Canvas(str(OUT),pagesize=(960,540))
c.setTitle('Monster Clash — Final Project Presentation')
c.setAuthor('Aibyn Samat, Diyar Sekezhanov, Dias Zhakin')
def title(text):
    c.setFillColorRGB(0,0,0);c.setFont('Bold',32);c.drawString(52,474,text)
def para(text,x,y,w=850,size=22):
    p=Paragraph(text,ParagraphStyle('p',fontName='Body',fontSize=size,leading=size*1.3))
    _,h=p.wrap(w,500);p.drawOn(c,x,y-h);return y-h
def slide():c.showPage()

title('MONSTER CLASH')
para('A 3D turn-based roguelike in C++ and OpenGL',52,420,size=28)
para('Computer Graphics Fundamentals<br/>Astana IT University · SE-2415',52,327,size=21)
para('Aibyn Samat<br/>Diyar Sekezhanov<br/>Dias Zhakin',52,225,size=23)
para('Instructor: Saleshova Sabina<br/>2026',610,147,w=300,size=19)
slide()

title('Explore, fight, upgrade, repeat')
y=para('We built a small arena game around a complete playable loop.',52,420)
for text in ['Explore with WASD and orbit the camera with Q/E.',
             'Approach a monster to start a turn-based battle.',
             'Attack, spend mana on a skill, or defend to recover mana.',
             'Win upgrades and clear floors; restart after defeat.']:
    y=para(text,70,y-24,size=23)
slide()

title('Three shader effects in one scene')
im=Image.open(HERE/'build_capture.png').convert('L')
c.drawImage(ImageReader(im),52,110,width=620,height=348.75)
para('<b>R</b> — player rim<br/><br/><b>T</b> — enemy dissolve<br/><br/><b>B</b> — PCF shadows',725,415,w=190,size=22)
para('Screenshot from our build. F2 repeats this scene; P freezes the animation.',52,91,size=18)
slide()

title('Dissolve uses the same mask in both passes')
para('Inputs: model transforms + shared enemy origin + progress',52,426,size=23)
for x,label,body in [(52,'Light-space depth','Vertex shader → light clip space<br/>Fragment shader → discard mask<br/>Output → depth texture'),
                      (500,'Camera-space colour','Vertex shader → camera clip space<br/>Fragment shader → same discard<br/>Output → lit colour + bright edge')]:
    c.rect(x,185,408,177);para('<b>'+label+'</b>',x+18,343,w=375,size=23);para(body,x+18,295,w=375,size=21)
para('mask = noise3D((worldPosition − enemyOrigin) × 5 + offset)<br/>Discard when mask &lt; progress; remove everything at progress = 1.',52,153,size=21)
slide()

title('Lighting and animation respond to input')
y=para('<b>Rim:</b> (1 − max(dot(N, V), 0))<super>4.5</super> × 0.21',52,425,size=25)
y=para('<b>Shadows:</b> directional-light depth map, depth bias and 3 × 3 PCF.',52,y-30,size=23)
y=para('<b>Idle:</b> sine-based squash/stretch, sway and breathing on model subtrees.',52,y-30,size=23)
y=para('<b>J/L</b> rotate the sun. <b>P</b> freezes gameplay and all animation.<br/><b>H/F1</b> shows controls. <b>G</b> enables the lighter rendering preset.',52,y-34,size=23)
slide()

title('Performance and validation')
rows=list(csv.reader((ROOT/'output/benchmark.csv').open()))
para('Debug x64 · 1280 × 720 · RTX 3050 Laptop GPU<br/>Fixed scene, 12 warm-up + 90 measured frames per mode.',52,426,size=21)
y=322
c.setFont('Bold',22);c.drawString(72,y,'Mode');c.drawString(570,y,'Average FPS')
c.line(52,y-15,908,y-15)
for row in rows[3:]:
    y-=42;c.setFont('Body',22);c.drawString(72,y,row[0]);c.drawString(610,y,f'{float(row[5]):.1f}')
para('Wall-clock FPS includes glFinish and swaps. The G preset meets 30 FPS locally.<br/>Five regression programs passed, including a real-driver dissolve/depth test.',52,75,size=18)
slide()

title('Team modules and live demonstration')
para('<b>Aibyn Samat:</b> gameplay, enemies, dissolve, animation and report.<br/><br/><b>Diyar Sekezhanov:</b> player, arena and rim lighting.<br/><br/><b>Dias Zhakin:</b> shadows, HUD, particles and hit feedback.',52,423,size=22)
para('Live sequence: F2 preview → T/R/B comparison → J/L light → P pause<br/>→ F2 play → one battle → upgrade.',52,190,size=23)
para('Original procedural assets; library and asset licenses, references and AI assistance are documented in the report. Every teammate should be ready to explain and modify the shaders.',52,92,size=17)
slide()
c.save()
print(OUT)
