"""Create the source-only submission ZIP; PDFs remain separate deliverables."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

ROOT=Path(__file__).resolve().parent
OUT=ROOT/'output/Monster_Clash_Source.zip'
OUT.parent.mkdir(exist_ok=True)
files=[]
for folder in ['Game','Modules','Tests','shaders','Assets/generated','Licenses']:
    files += [p for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
for folder in ['GL','glad','glm','KHR','STB']:
    files += [p for p in (ROOT/'Libraries/include'/folder).rglob('*') if p.is_file()]
for name in ['.gitattributes','.gitignore','Main.cpp','glad.c','OpenJL.vcxproj','OpenJL.vcxproj.filters',
             'FinalProjectGraphics.slnx','run-game.cmd','README.md','THIRD_PARTY.md',
             'SUBMISSION_CHECKLIST.md','package_source.py','freeglutd.dll',
             'Libraries/lib/freeglut.lib','Libraries/lib/freeglutd.lib',
             'output/benchmark.csv','output/pdf/build_report.py',
             'output/pdf/report_metadata.json','output/pdf/build_capture.png',
             'output/pdf/build_presentation.py','output/pdf/Defence_Notes.md']:
    p=ROOT/name
    if p.exists(): files.append(p)
with ZipFile(OUT,'w',ZIP_DEFLATED,compresslevel=9) as z:
    for p in sorted(set(files)):
        z.write(p,Path('Monster_Clash')/p.relative_to(ROOT))
print(f'{OUT}: {len(set(files))} files, {OUT.stat().st_size:,} bytes')
