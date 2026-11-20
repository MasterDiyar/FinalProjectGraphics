"""Rebuild the current colour presentation PDF from the reviewed PowerPoint."""
from pathlib import Path
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[2]
NODE=Path.home()/'.cache/codex-runtimes/codex-primary-runtime/dependencies/node/bin/node.exe'
DECK=ROOT/'output/presentation/Monster_Clash_Defence_Final.pptx'
if not DECK.exists():
    subprocess.run([str(NODE),str(ROOT/'output/presentation/build_deck.mjs'),DECK.name],cwd=ROOT,check=True)
subprocess.run([str(NODE),str(ROOT/'output/presentation/render_deck.mjs')],cwd=ROOT,check=True)
subprocess.run([sys.executable,str(ROOT/'output/presentation/export_pdf.py')],cwd=ROOT,check=True)
