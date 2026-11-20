# Monster Clash — final defence

- `Monster_Clash_Presentation.pdf` — presentation for submission, 10 pages.
- `Monster_Clash_Defence_Final.pptx` — editable PowerPoint, with speaker notes,
  an editable FPS chart and links to the team video.
- Slides 1–9 are the five-minute talk. Slide 10 is backup material for questions;
  it is hidden in the PowerPoint slide show and included in the PDF.

The slide text is in English, matching the report and course. Suggested speakers
and timings are in the PowerPoint notes: Diyar introduces the game and rim
lighting; Aibyn covers gameplay, dissolve, animation and the wrap-up; Dias covers
shadow mapping, rendering passes and performance. These are speaking suggestions,
not additional claims about code ownership.

## Visual evidence

Video: https://youtu.be/-plQpir5YrU

- 0:53 — exploration frame used on the cover.
- 1:19 — a turn-based battle.
- 1:59 — the next floor and new enemies.

The recording frames are cropped to the game window and retain their original
colours. Controlled on/off shader comparisons come from the same frozen scene of
our measured build, rather than from unmatched video frames. The FPS chart uses
`output/benchmark-release-2026-11-20.csv`, measured in Release x64 on 20 November
2026: 141.1 FPS with all effects and approximately 144 FPS in the other modes.
Ordinary gameplay uses a separate 16 ms GLUT timer (approximately 60 FPS). An
explicit V-Sync setting has not been verified. The old Debug CSV is preserved. Sources, equations, qualifications and references are in the notes.

The PDF contains still frames with clickable video links. It does not embed a
moving video. Internet access is needed to open YouTube. For the live demo, use
F2, compare T/R/B, rotate the sun with J/L, then freeze an effect with P. F2 resets
the current run when entering or exiting the preview.

The full recording is 2:13. The requirements specify a 60–120 second backup;
an excerpt from 0:13 to 2:13 is 120 seconds. Use a trimmed copy for that strict
duration requirement rather than describing the full recording as two minutes.

## Design and rebuilding

This is an original layout, not a Canva template. Colours are navy, lime, mint
and orange; game images retain colour. There are no repeated page footers.

Authoring source: `build_deck.mjs`; requires the bundled Artifact Tool and skill
paths shown in the source. Pass a new PPTX filename as the first argument for a
revision; the finalizer preserves previously reviewed output files.
`render_deck.mjs` renders the reviewed PowerPoint, and `export_pdf.py` creates
the matching PDF with clickable links. The existing entry point
`python output/pdf/build_presentation.py` runs these two steps.

The separate report remains typographically black and white, with its screenshot
restored to full colour. It is at `output/pdf/Monster_Clash_Project_Report.pdf`.
