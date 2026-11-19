# Final submission — Monster Clash

Team: Aibyn Samat, Diyar Sekezhanov, Dias Zhakin · SE-2415.
Instructor: Saleshova Sabina. Free-choice topic: **Monster Clash**.

## Implemented common requirements

- C++ and OpenGL 3.3 core; no full game engine.
- Default 1280×720 window, resize-safe perspective and Q/E orbit camera.
- Vertex/fragment programs loaded from `shaders/`; console compile/link errors.
- Depth test and outward CCW back-face culling; screen-space HUD disables culling.
- H/F1 on-screen controls; P pauses gameplay and animation.
- R rim, T dissolve, B shadow on/off comparison; toggle states shown on screen.
- J/L rotate directional light; G switches full/low quality.
- Six-second opening shader preview; F2 repeats it or returns to play.
- Team names, game title, floor/kill status and FPS displayed on screen.
- Explore/battle/upgrade/defeat/restart loop and continuous enemy animation.
- Original procedural models/textures; active texture source and CC0 notice supplied.
- README, CLI Visual Studio build, tests and repeatable FPS benchmark.
- PDF report: pipeline diagram, actual equations, controls, measurements,
  checklist, references, contribution table and AI assistance disclosure.

## Required before upload

- Record a 60–120 second demo showing gameplay **and code**, make it public on
  YouTube and put its real URL in `output/pdf/report_metadata.json`; regenerate
  the report with `output/pdf/build_report.py`. A placeholder is not a valid URL.
- Practise the supplied five-minute presentation (output/pdf/Monster_Clash_Presentation.pdf).
- Upload presentation PDF, report PDF and source ZIP as three separate files.
- Run on the actual lab PC. Use G if FPS falls below 30; local benchmark results
  do not certify the lab computer. Every teammate should practise explaining
  and changing the shaders at the defence.

Section 6 is absent from the supplied requirements. The team confirmed that the
topic is free-choice; this submission checks the common requirements and the
implemented Monster Clash features rather than inventing an assigned topic list.
