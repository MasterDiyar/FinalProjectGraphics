import fs from 'node:fs/promises';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import {Presentation,PresentationFile} from 'file:///C:/Users/paradise/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/@oai/artifact-tool/dist/artifact_tool.mjs';
process.env.RUNTIME_NODE_MODULES='C:/Users/paradise/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules';
const ROOT=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'../..');
const OUT=path.join(ROOT,'output/presentation');
const TMP=path.join(ROOT,'tmp/presentation/local');
const A=path.join(OUT,'assets');
const imageBlobs=Object.fromEntries(await Promise.all((await fs.readdir(A)).filter(n=>n.endsWith('.png')).map(async n=>[n,new Uint8Array(await fs.readFile(path.join(A,n)))])));
const p=Presentation.create({slideSize:{width:1280,height:720}});
const C={dark:'#10212E',ink:'#16303D',muted:'#53707A',white:'#F7F9F5',lime:'#B9F578',mint:'#67D7C5',orange:'#FFA367',line:'#D9E3DD'};
const sources='Source code: https://github.com/MasterDiyar/FinalProjectGraphics/tree/feature/battle\nVideo: https://youtu.be/-plQpir5YrU';
const links=[];
function shape(s,x,y,w,h,fill,geometry='rect',stroke='none',sw=0){return s.shapes.add({geometry,position:{left:x,top:y,width:w,height:h},fill,line:{fill:stroke,width:sw,style:'solid'}});}
function text(s,t,x,y,w,h,size=28,color=C.ink,bold=false){const q=shape(s,x,y,w,h,'none','textbox');q.text=t;q.text.insets=0;q.text.style={fontFamily:'Arial',fontSize:size,color,bold,verticalAlignment:'top',wrap:true};return q;}
function slide(title,dark=false){const s=p.slides.add();s.background.fill=dark?C.dark:C.white;text(s,title,64,48,1150,98,48,dark?C.white:C.ink,true);return s;}
function img(s,file,x,y,w,h,href){s.images.add({blob:imageBlobs[file],contentType:'image/png',position:{left:x,top:y,width:w,height:h},fit:'contain',alt:file,...(href?{hyperlink:{uri:href,isExternal:true}}:{})});if(href)links.push({slide:p.slides.items.length,x,y,w,h,href});}
function link(s,t,x,y,w,sec){const href=`https://www.youtube.com/watch?v=-plQpir5YrU&t=${sec}s`;const q=text(s,t,x,y,w,30,20,C.muted);q.hyperlink={uri:href,isExternal:true};links.push({slide:p.slides.items.length,x,y,w,h:30,href});}
function note(s,t){s.speakerNotes.text=t+'\n\n'+sources;}
function box(s,t,x,y,w,h,fill=C.dark,color=C.white,size=24){shape(s,x,y,w,h,fill);text(s,t,x+22,y+20,w-44,h-36,size,color,true);}
function arrow(s,x,y,w=52,color=C.muted){shape(s,x,y,w,25,color,'rightArrow');}

// 1 — A cover built around the real recording rather than decorative stock art.
{
const s=p.slides.add();s.background.fill=C.dark;
text(s,'MONSTER\nCLASH',64,75,580,205,92,C.white,true);
text(s,'A 3D turn-based game\nin C++ and OpenGL',68,310,540,100,34,C.lime);
img(s,'video-53.png',620,146,596,335);
text(s,'Computer Graphics Fundamentals\nSE-2415 · Astana IT University',68,465,520,68,23,C.white);
text(s,'Aibyn Samat · Diyar Sekezhanov · Dias Zhakin',68,599,1100,36,23,C.white);
text(s,'Instructor: Saleshova Sabina',68,644,1000,30,20,C.mint);
note(s,'0:00–0:25 · Diyar\nMonster Clash is our free-choice final project. We wanted a small game where graphics effects support an actual game loop. We use C++ and OpenGL 3.3 core, with FreeGLUT, GLAD, GLM and stb_image. The screenshots on these slides come from our recording and our own tested build. The video opens by clicking the image.\nCover image: recording at 0:53.');
}
// 2 — Recording establishes that the project is playable, with visible status.
{
const s=slide('Explore, fight, and reach the next floor');
img(s,'video-79.png',64,170,550,309,'https://www.youtube.com/watch?v=-plQpir5YrU&t=79s');
img(s,'video-119.png',666,170,550,309,'https://www.youtube.com/watch?v=-plQpir5YrU&t=119s');
link(s,'01:19 · Turn-based battle ↗',64,493,550,79);link(s,'01:59 · Next floor and new enemies ↗',666,493,550,119);
for(const [i,t] of ['Explore','Battle','Upgrade','Next floor'].entries()){box(s,t,64+i*296,563,248,70,i===1?C.ink:C.line,i===1?C.white:C.ink,25);if(i<3)arrow(s,321+i*296,586,30);}
text(s,'Goal: clear floors. Score: kills and floor reached. Defeat → restart in the same process.',64,660,1150,30,20,C.muted);
note(s,'0:25–1:00 · Aibyn\nWe move with WASD and rotate the camera with Q and E. Touching a living monster starts a battle. Attack is free, skill spends mana, and defend restores mana while reducing the next hit. Winning gives an upgrade, and clearing all three enemies starts a stronger floor. On defeat, Space starts a new run, so the program can be played continuously.\nActual recording frames: 1:19 and 1:59.');
}
// 3 — Two images share scene, camera and dissolve time; only T changes.
{
const s=slide('Dissolve: the last hit removes the monster',true);
img(s,'dissolve-off.png',64,165,550,270);img(s,'dissolve-on.png',666,165,550,270);
text(s,'T OFF · intact surface',64,452,550,35,26,C.white,true);text(s,'T ON · holes + orange edge',666,452,550,35,26,C.orange,true);
text(s,'mask = noise3D((world − enemyOrigin) × 5 + offset)',64,525,1150,44,29,C.lime);
text(s,'Discard if mask < progress. A 0.09-wide band colours the surviving edge.\nThe same discard mask removes the matching parts of the shadow.',64,592,1140,78,26,C.white);
note(s,'1:00–1:40 · Aibyn\nThis is a controlled comparison from the same scene and moment. A shared enemy origin makes all cube parts sample one continuous 3D noise field. Progress rises from zero to one over 0.9 seconds. The fragment shader discards fragments below the threshold, and colours the narrow surviving band orange. The shadow pass uses the same mask, so we do not leave a complete shadow after the model disappears. T bypasses the effect; P freezes the timer.\nSources: shaders/enemy_dissolve.vert, shaders/enemy_dissolve.frag, Game/DissolveState.h. Comparisons: benchmark-3 and benchmark-0, 19 November 2026.');
}
// 4 — An intentionally restrained rim effect; do not describe it as bloom.
{
const s=slide('Rim lighting separates the player from the arena');
img(s,'hero-2.png',100,168,420,350);img(s,'hero-0.png',700,168,420,350);
text(s,'R OFF',220,531,240,40,26,C.muted,true);text(s,'R ON',830,531,240,40,26,C.ink,true);
text(s,'rim = (1 − max(N · V, 0))^4.5 × 0.21',64,604,1150,40,31,C.ink,true);
text(s,'R OFF uses the world shader; R ON uses the player shader. No bloom pass.',64,660,1150,30,22,C.muted);
note(s,'1:40–2:10 · Diyar\nRim lighting adds a small blue highlight where the surface normal becomes almost perpendicular to the viewing direction. We use the actual camera position to compute V, normalise both vectors, raise the rim term to power 4.5 and scale it by 0.21. This is a surface colour contribution, not a bloom glow. R OFF selects the world shader while R ON selects the player shader, so base brightness also changes in this implementation; this is not an isolated rim-intensity experiment. The player uses the normal depth shader in the shadow pass.\nSources: Modules/PlayerNode.cpp, shaders/player_rim.vert and player_rim.frag. Image comparison: benchmark-2 versus benchmark-0.');
}
// 5 — Shadow comparison with the same depth-map sampling scene.
{
const s=slide('Shadow mapping gives the blocks depth');
img(s,'arena-1.png',64,169,550,288);img(s,'arena-0.png',666,169,550,288);
text(s,'B OFF · diffuse light only',64,478,550,35,26,C.muted,true);text(s,'B ON · sun depth + 3 × 3 PCF',666,478,550,35,26,C.ink,true);
shape(s,151,306,74,29,'none','ellipse',C.orange,3);shape(s,753,306,74,29,'none','ellipse',C.orange,3);
text(s,'1. Render scene depth from the sun.\n2. Compare each camera fragment with that depth.',64,553,770,95,27,C.ink);
box(s,'J / L\nRotate the sun',942,548,274,112,C.ink,C.lime,27);
note(s,'2:10–2:45 · Dias\nShadowMap creates a 2048 by 2048 depth texture. An orthographic camera looks at the arena from the sun direction. In the colour pass we transform the fragment into light space and compare its depth with the stored depth. We average nine nearby comparisons to soften the edge. Normal offset, a small bias and polygon offset reduce self-shadowing. J and L rotate the directional light; B skips the shadow pass and comparison.\nSources: Game/ShadowMap.h and shaders/world.frag. Controlled image comparison: benchmark-1 versus benchmark-0.');
}
// 6 — Editable diagram; actual passes, shader stages, inputs and outputs.
{
const s=slide('One scene graph, two world passes, one HUD',true);
box(s,'INPUTS\nMeshes · transforms\nCamera · sun\nEffect state',64,178,330,170,C.lime,C.dark,24);
arrow(s,424,245,58,C.mint);
box(s,'DEPTH PASS\nVertex: light clip space\nFragment: depth / discard',515,174,300,170,'#213D4D',C.white,22);
box(s,'COLOUR PASS\nVertex: camera clip space\nFragment: light + effects',896,174,320,170,'#213D4D',C.white,22);
arrow(s,831,245,48,C.mint);
text(s,'Shadow depth texture',540,382,290,40,25,C.mint,true);
shape(s,1050,358,3,80,C.mint);shape(s,1036,423,30,40,C.mint,'downArrow');
box(s,'HUD → FINAL FRAME\nNames · score · help · toggle states',706,483,510,113,C.lime,C.dark,27);
text(s,'The dissolve mask is shared by both world passes.\nDepth + back-face culling apply to 3D meshes.\nThe HUD disables both.',64,493,587,130,24,C.white);
note(s,'2:45–3:25 · Dias\nNode stores transforms and children; MeshNode draws indexed geometry. GameApp updates the scene, renders a light-space depth pass, then renders the camera-space colour pass using that texture. Finally HudNode draws in a screen-space orthographic projection. The same dissolve rule is used in the two world passes, while player rim is only used in the colour pass. The 3D scene has depth testing and outward CCW back-face culling. We disable both for the HUD.\nSource: Game/GameApp.cpp, Modules/Node.h, Modules/MeshNode.cpp. OpenGL stage definitions: Segal and Akeley (2010), OpenGL 3.3 core specification.');
}
// 7 — Continuous animation is separated from gameplay anchors.
{
const s=slide('The arena keeps moving while the player waits');
const items=[['slime.png','SLIME','1.2 s · squash / stretch'],['goblin.png','GOBLIN','1.7 s · sway + small bob'],['golem.png','GOLEM','2.4 s · slow breathing']];
for(let i=0;i<3;i++){const x=64+i*396;img(s,items[i][0],x,180,360,240);text(s,items[i][1],x,449,360,40,27,C.ink,true);text(s,items[i][2],x,501,360,70,23,C.muted);}
text(s,'wave = sin(2πt / period)',64,602,800,43,33,C.ink,true);
text(s,'Animation changes the model subtree. Encounter positions stay fixed. P freezes all loops.',64,660,1150,30,21,C.muted);
note(s,'3:25–3:50 · Aibyn\nEach enemy type has a continuous sine-based idle loop. Slime squashes and stretches, goblin sways and bobs, and golem breathes slowly. Each monster has a different initial phase. We animate the model below the combat anchor, so idle movement does not move encounter positions or health bars. Attack lunges and hit flashes are separate offsets, and P freezes the animation.\nSource: Game/EnemyNode.cpp. Slime/golem images from our build; goblin image from recording at 0:58.');
}
// 8 — Native editable chart; actual measured snapshot, not an estimate.
{
const s=slide('39 FPS with every effect enabled');
text(s,'Debug x64 · 1280 × 720 · RTX 3050 Laptop GPU',64,149,1140,38,25,C.muted);
const vals=[39.1864,63.3765,43.2893,42.2017,62.4371];
s.charts.add('bar',{position:{left:64,top:208,width:805,height:393},categories:['All effects','Shadows off','Rim off','Dissolve off','Low quality'],series:[{name:'Average FPS',values:vals,fill:'#329E82'}],hasLegend:false,barOptions:{direction:'bar',grouping:'clustered',gapWidth:80},chartFill:C.white,plotAreaFill:C.white,xAxis:{textStyle:{typeface:'Arial',fontSize:21,fill:C.ink}},yAxis:{textStyle:{typeface:'Arial',fontSize:18,fill:C.muted},majorGridlines:{fill:C.line,width:1},numberFormatCode:'0'},dataLabels:{showValue:true,position:'outEnd',numberFormatCode:'0.0',textStyle:{typeface:'Arial',fontSize:22,fill:C.ink,bold:true}}});
text(s,'30 FPS',936,252,280,65,53,C.ink,true);text(s,'course target',940,325,260,40,25,C.muted);
text(s,'G → low quality\n≈ 62 FPS',936,427,280,110,29,C.ink,true);
text(s,'19 Nov build · 12 warm-up + 90 measured frames per mode. Wall time includes swaps and glFinish.',64,652,1140,46,20,C.muted);
note(s,'3:50–4:20 · Dias\nThese are actual measurements, not values read from a single HUD frame. Every comparison uses the same frozen scene and camera, with twelve warm-up frames and ninety timed frames. Full quality averages 39.2 FPS on the tested laptop. Disabling shadows has the largest improvement. G selects the low-quality preset, which skips shadow rendering and averages 62.4 FPS. These are wall-clock frame rates including swaps and glFinish, not isolated GPU timings. The lab machine should still be checked.\nData: output/benchmark.csv, measured 19 November 2026 before subsequent road-shader experiments. Five regression programs passed for that tested build.');
}
// 9 — Ownership, live demo and a real linked recording.
{
const s=slide('Three modules, one playable project',true);
const rows=[['Aibyn Samat','Gameplay + enemies','Dissolve · idle loops · tests · report'],['Diyar Sekezhanov','Player + arena','Rim lighting · player and arena setup'],['Dias Zhakin','Shadows + feedback','Depth pass · HUD · particles · hit feedback']];
for(let i=0;i<3;i++){const y=182+i*111;text(s,rows[i][0],64,y,355,43,31,C.lime,true);text(s,rows[i][1],444,y,760,38,29,C.white,true);text(s,rows[i][2],444,y+47,770,40,23,C.white);}
box(s,'LIVE DEMO: F2 → T / R / B → J / L → P → return to play',64,542,1152,73,C.lime,C.dark,27);
const q=text(s,'Open the demonstration video ↗',64,648,1100,35,23,C.mint,true);q.hyperlink={uri:'https://youtu.be/-plQpir5YrU',isExternal:true};links.push({slide:9,x:64,y:648,w:1100,h:35,href:'https://youtu.be/-plQpir5YrU'});
note(s,'4:20–5:00 · Aibyn, then live demo\nWe divided ownership across gameplay and enemies, player and arena, and shadows and feedback. We used Git to share changes and tests to check the integrated game. For the demo, show the F2 preview, toggle dissolve, rim and shadows, rotate the sun, and pause midway through an animation. Return to gameplay and show a battle if time allows. Each of us should be ready to explain and change the shader code.\nAI assistance is acknowledged in the report: OpenAI ChatGPT/Codex assisted substantial code and report drafts. Active build assets are procedural, with CC0 textures; helper library licenses are included.');
}
// 10 — Backup material for questions, excluded from the five-minute timing.
{
const s=slide('Backup: controls, code and references');s.hidden=true;
text(s,'WASD / Q E\nMove / orbit camera\n\n1 2 3 / mouse\nBattle or upgrade\n\nH or F1 / P\nHelp / pause',64,168,445,380,28,C.ink);
text(s,'R / T / B\nRim / dissolve / shadows\n\nJ L / G\nSun direction / low quality\n\nF2 / Space\nPreview / restart after defeat',650,168,565,380,28,C.ink);
text(s,'References: OpenGL 3.3 core specification (Segal & Akeley, 2010); project source and team video.\nGLAD, FreeGLUT, GLM and stb_image. Asset licenses and AI assistance: see written report.',64,585,1152,80,21,C.muted);
note(s,'Backup only — not part of the five-minute slide sequence.\nReferences (APA 7):\nSegal, M., & Akeley, K. (2010). The OpenGL graphics system: A specification (Version 3.3, core profile). Khronos Group. https://registry.khronos.org/OpenGL/specs/gl/glspec33.core.pdf\nMonster Clash team. (2026). Monster Clash [Source code]. https://github.com/MasterDiyar/FinalProjectGraphics\nAspect of the Ice. (2026). Computer graphics final [Video]. YouTube. https://youtu.be/-plQpir5YrU\nOpenAI. (2026). ChatGPT (Codex) [Large language model]. https://chatgpt.com/\nFootage is captured from the team recording; on/off comparisons are from the same frozen scene of the measured build. These are separate evidence sources, not one camera-matched recording.');
}
await fs.mkdir(TMP,{recursive:true});await fs.mkdir(OUT,{recursive:true});
await (await PresentationFile.exportPptx(p)).save(path.join(TMP,'candidate.pptx'));
for(let i=0;i<p.slides.items.length;i++){const b=await p.export({slide:p.slides.items[i],format:'png',scale:1});await fs.writeFile(path.join(TMP,`slide-${String(i+1).padStart(2,'0')}.png`),new Uint8Array(await b.arrayBuffer()));}
await fs.writeFile(path.join(TMP,'links.json'),JSON.stringify(links));
console.log(`Authored and rendered ${p.slides.items.length} slides`);
try {const pdf=await p.export({format:'pdf'});await fs.writeFile(path.join(OUT,'Monster_Clash_Presentation.pdf'),new Uint8Array(await pdf.arrayBuffer()));console.log('PDF exported');}catch(e){console.log('PDF export fallback required:',e.message);}
const {finalizePresentation}=await import('file:///C:/Users/paradise/.codex/plugins/cache/openai-primary-runtime/presentations/26.1119.11507/skills/presentations/container_tools/artifact_tool_utils.mjs');
const result=await finalizePresentation({workspaceDir:ROOT,candidatePath:path.join(TMP,'candidate.pptx'),finalPath:path.join(OUT,process.argv[2] ?? 'Monster_Clash_Defence_Rebuilt.pptx'),pythonExecutable:'C:/Users/paradise/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe',integrityValidatorPath:'C:/Users/paradise/.codex/plugins/cache/openai-primary-runtime/presentations/26.1119.11507/skills/presentations/container_tools/inspect_presentation_package_integrity.py',layoutValidatorPath:'C:/Users/paradise/.codex/plugins/cache/openai-primary-runtime/presentations/26.1119.11507/skills/presentations/container_tools/inspect_presentation_layout_geometry.py',layoutArgs:['--expected-slide-size-emu','12192000,6858000','--validate-bullet-geometry','--validate-heading-fit'],explicitTotalSlideCount:10,materializeLiteralChartWorkbooks:true,requiredNativeChartOwnerSlides:[8],fontPolicy:{basis:'design',families:['Arial']},verifyArtifactToolImport:true,receiptPath:path.join(TMP,`validation-${Date.now()}.json`)});
console.log(JSON.stringify(result));


