from pathlib import Path
import json, math
from xml.sax.saxutils import escape
from reportlab.pdfgen import canvas
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, PageBreak, Table, TableStyle, Flowable, Image
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_CENTER, TA_JUSTIFY
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
META = json.loads((HERE / 'report_metadata.json').read_text(encoding='utf-8'))
FONT = Path('C:/Windows/Fonts')
for name, file in [('Body','times.ttf'),('Bold','timesbd.ttf'),('Italic','timesi.ttf')]:
    pdfmetrics.registerFont(TTFont(name, str(FONT / file)))
NAVY = colors.black
TEAL = colors.black
PALE = colors.white
GRAY = colors.black
W = 483
styles = {
    'body': ParagraphStyle('body', fontName='Body', fontSize=12, leading=16, spaceAfter=10, alignment=TA_JUSTIFY),
    'h1': ParagraphStyle('h1', fontName='Bold', fontSize=16, leading=21, textColor=NAVY, spaceAfter=16),
    'h2': ParagraphStyle('h2', fontName='Bold', fontSize=12, leading=16, textColor=TEAL, spaceBefore=8, spaceAfter=6),
    'small': ParagraphStyle('small', fontName='Body', fontSize=10, leading=13, textColor=GRAY, spaceAfter=8),
    'cell': ParagraphStyle('cell', fontName='Body', fontSize=10.5, leading=13),
    'th': ParagraphStyle('th', fontName='Bold', fontSize=10.5, leading=13, textColor=colors.black),
    'center': ParagraphStyle('center', fontName='Body', fontSize=12, leading=18, alignment=TA_CENTER),
    'code': ParagraphStyle('code', fontName='Body', fontSize=11, leading=15, leftIndent=12, spaceBefore=4, spaceAfter=12),
}
story=[]
def p(text, style='body'): return Paragraph(text, styles[style])
def add(text, style='body'): story.append(p(text,style))
def h(text): add(text,'h2')
def page(title):
    if story: story.append(PageBreak())
    add(title,'h1')
def table(headers, rows, widths):
    data=[[p(x,'th') for x in headers]]+[[p(str(x),'cell') for x in row] for row in rows]
    t=Table(data,colWidths=widths,hAlign='LEFT')
    t.setStyle(TableStyle([('VALIGN',(0,0),(-1,-1),'TOP'),
        ('LEFTPADDING',(0,0),(-1,-1),9),('RIGHTPADDING',(0,0),(-1,-1),9),
        ('TOPPADDING',(0,0),(-1,-1),6),('BOTTOMPADDING',(0,0),(-1,-1),6),
        ('LINEABOVE',(0,0),(-1,0),0.6,colors.black),
        ('LINEBELOW',(0,0),(-1,0),0.5,colors.black),
        ('LINEBELOW',(0,-1),(-1,-1),0.5,colors.black)]))
    story.extend([t,Spacer(1,10)])
def note(title, text):
    add(title, 'h2')
    add(text)

class Loop(Flowable):
    def __init__(self): Flowable.__init__(self); self.width=W; self.height=85
    def draw(self):
        c=self.canv
        for i,label in enumerate(['Explore','Battle','Reward','Next floor']):
            x=i*123
            c.setFillColor(PALE); c.setStrokeColor(TEAL); c.roundRect(x,34,111,32,5,fill=1,stroke=1)
            c.setFillColor(NAVY); c.setFont('Bold',10); c.drawCentredString(x+55,46,label)
            if i<3:
                c.line(x+112,50,x+121,50); c.line(x+117,53,x+121,50); c.line(x+117,47,x+121,50)
        c.setFont('Body',9); c.setFillColor(GRAY)
        c.drawString(0,12,'Defeat -> restart with a session-only attack bonus. Reward -> explore if enemies remain.')

def footer(c, doc):
    pass

# 1. Academic cover.
story.append(Spacer(1,28))
add('ASTANA IT UNIVERSITY','center')
story.append(Spacer(1,52))
add('FINAL PROJECT REPORT','center')
story.append(Spacer(1,18))
story.append(Paragraph('MONSTER CLASH',ParagraphStyle('cover',fontName='Bold',fontSize=26,leading=32,alignment=TA_CENTER,textColor=NAVY)))
add('A 3D Turn-Based Roguelike in C++ and OpenGL','center')
story.append(Spacer(1,23))
add('Course: Computer Graphics Fundamentals','center')
story.append(Spacer(1,58))
story.append(Spacer(1,10))
for line in ['Submitted by:'] + META['students'] + ['', 'Group: '+META['group'], 'Instructor: '+META['instructor']]:
    story.append(Paragraph(line or '&nbsp;',ParagraphStyle('details',fontName='Body',fontSize=12,leading=19,leftIndent=240)))
story.append(Spacer(1,38))
add('Topic: Monster Clash (free choice)','center')
add('Astana, 2026','center')
story.append(Spacer(1,25))

# 2. Overview.
page('1. Project overview')
h('Abstract')
add('Monster Clash is a 3D turn-based game that we built in C++ and OpenGL. The player explores an arena and starts a battle by approaching a monster. In battle, the player can attack, use a skill or defend. Winning gives an upgrade, and defeating all enemies opens a harder floor. Losing starts a new run with a small attack bonus that remains until the program is closed.')
add('We used the project to work with 3D transformations, textures, lighting and animation. The game includes shadow mapping, rim lighting for the player, hit particles and a dissolve effect for defeated enemies. Monsters also have continuous idle animations. This report explains how these features work in the project build, with performance measurements updated on 20 November 2026.')
h('Project goals')
add('Our goal was to make a small playable game using the graphics techniques covered in the course. We needed a 3D scene, a camera, shaders, animation and an interface that responded to player input. We chose one arena and simple block models so that we could spend more time on these systems.')
add('The visual style was inspired by Roblox, while the turn-based combat was inspired by Pokemon. We added repeated runs, upgrades and stronger enemies on later floors to give the player a reason to keep playing.')
story.append(Loop())
add('Figure 1. Implemented gameplay loop. The next floor is entered only after all enemies are defeated and an upgrade is selected.','small')
h('Demonstration video')
if META.get('video_url'):
    u=escape(META['video_url']); add(f'YouTube demonstration: <link href="{u}" color="#000000">{u}</link>')
else:
    add('The public YouTube link will be added once the team has uploaded the demonstration video.')

# 3. Gameplay.
page('2. Gameplay and combat systems')
h('Exploration and encounters')
add('The player moves with WASD and rotates the camera with Q and E. Movement is limited to the arena, with a boundary of 9.5 units. We normalize movement input so that moving diagonally is not faster. When the player gets within 1.3 units of a living enemy, the game switches from exploration to battle.')
add('Each floor contains three randomly selected enemies. To keep them apart, we shuffle eight possible spawn zones and add an offset of up to 0.75 units. This keeps enemies at least 4.5 units apart and leaves room around the starting position.')
table(['Enemy','Base HP','Attack','Defense','Appearance'],[
 ['Slime','20','6','1','Low, wide body'],['Goblin','28','8','2','Head, ears and arms'],['Golem','40','7','4','Broad stone body'],
],[70,58,65,70,220])
h('Turn-based actions')
add('The three battle actions have different uses. Attack deals normal damage. Skill uses 3 MP and doubles the attack value. Defend reduces the next incoming damage by half and restores up to 2 MP. After an accepted action, the enemy responds after a 0.9-second delay. If the player cannot afford a skill, the turn is not spent. Input is blocked during the enemy turn.')
add('damage = max(1, attack - floor(defense / 2) + random[-2, 2])<br/>defended damage = max(1, floor(damage / 2))','code')
h('Progression and state transitions')
add('We use four main game states: Explore, Battle, Upgrade and Defeat. After a victory, the player recovers up to 3 MP and chooses one upgrade: +2 attack, +8 maximum HP with healing, or +4 maximum MP with refilling. Once all three enemies are defeated, selecting an upgrade starts the next floor. Enemy HP and attack are multiplied by 1 + 0.15 x (floor - 1). Defense depends on the enemy type.')
add('After a defeat, the player returns to the first floor and loses the upgrades collected during that run. The starting attack increases by one as a session bonus. We have not added a save file, so this bonus disappears when the game closes. The current version has no final boss or end-of-game victory screen.')

# 4. Architecture.
page('3. Software architecture')
add('We split the code into classes so that gameplay, drawing and input would be easier to maintain. Main.cpp only calls runGame. GameApp.cpp creates the window, registers the FreeGLUT callbacks, builds the scene and coordinates the camera and combat. The smaller classes handle individual parts of the game.')
table(['Component','Responsibility'],[
 ['Node / MeshNode','Parent-child ownership, local/world transforms, visual offsets and indexed mesh rendering.'],
 ['ArenaNode / PlayerNode','Textured arena assembly; player geometry, movement and rim-light rendering.'],
 ['EnemyNode','Fighter statistics, model variants, HP bar, idle animation and death/dissolve lifecycle.'],
 ['HudNode / HudState','Screen-space interface; a value snapshot of game state; scaled button hit testing.'],
 ['Battle / ParticleNode','Turn rules and damage; pooled cube particles for hit feedback.'],
 ['ShadowMap / DissolveShader','Sun depth target and shader setup; file-based compilation and error reporting for dissolve.'],
],[139,344])
h('Scene hierarchy')
add('Root scene<br/>&nbsp;&nbsp;World scene: ArenaNode, PlayerNode, player HP bar,<br/>&nbsp;&nbsp;three EnemyNode instances and ParticleNode<br/>&nbsp;&nbsp;HudNode: final screen-space rendering pass<br/><br/>EnemyNode<br/>&nbsp;&nbsp;Body/animation anchor -> selected block-model subtree<br/>&nbsp;&nbsp;Health-bar mesh','code')
add('Objects are arranged in a parent-child hierarchy. A child inherits the world transform of its parent, then applies its own translation, rotation and scale. For enemies, the position used by combat belongs to an anchor node. Idle animation changes the model below that anchor. This lets a monster stretch or sway without changing where an encounter starts.')
h('Interface separation')
add('Before drawing the interface, we copy the relevant game values into HudState. HudNode uses that copy to show health, mana, messages and available actions. Mouse input is converted to an 800 x 600 logical canvas so that buttons still work after resizing. Mouse clicks and keyboard shortcuts use the same action function.')

# 5. Graphics.
page('4. Rendering and shader effects')
h('Rendering pipeline and assets')
add('We use OpenGL to draw the scene, GLAD to load its functions, FreeGLUT for the window and input, GLM for matrix calculations and stb_image for textures. GLFW is included in the repository but is not used by the current version. Meshes are stored in vertex and index buffers, and our shaders use GLSL 330.')
add('Each frame starts by updating the game and scene nodes. The world is first drawn into a depth map for the sun, then drawn in colour using that map for shadows. The HUD is drawn last. Grass, road and wall textures are applied to the arena. The player and monsters are built from cubes. We also have an OBJ loader, but the supplied hero.obj.disabled file is not loaded; the game uses the built-in character instead.')
h('Directional light and shadow mapping')
add('For the world, we combine ambient light with diffuse light from one directional source. Flat face normals are calculated from changes in world-space position. ShadowMap creates a 2048 x 2048 depth texture using an orthographic light camera. During the colour pass, the shader compares each fragment against the depth stored from the sun view.')
add('world colour *= 0.45 + 0.55 x max(dot(N, L), 0) x (1 - 0.8 x shadow)','code')
add('The shader samples a 3 x 3 area of the shadow map to soften the result. We use depth bias, a normal offset and polygon offset to reduce self-shadowing artefacts. These settings still need care because too much bias can make a shadow appear detached from an object.')
h('Player rim lighting')
add('The player uses player_rim.vert and player_rim.frag. These shaders add a coloured highlight near the edges of the character. The effect depends on the angle between the surface normal and the viewing direction. A power of 4.5 keeps the highlight narrow, and the final colour is clamped to the displayable range.')
add('rim = (1 - max(dot(N, V), 0))^4.5 x rimIntensity<br/>final colour = clamp(base colour + rimColour x rim, 0, 1)','code')
add('The rim effect is calculated on the character surface. We did not add a bloom pass. Separate hit flashes, particles and camera shake help show when an attack connects, while the actual damage is calculated by the battle system.')

# 6. Dissolve.
page('5. Procedural enemy dissolve')
add('We added the dissolve effect in enemy_dissolve.vert and enemy_dissolve.frag. After the last hit animation, the defeated monster gradually disappears over 0.9 seconds. Small holes spread across its surface, with an orange edge around them. The upgrade screen appears after the effect finishes.')
h('Shared coordinates across a hierarchical model')
add('A monster consists of several cube meshes. To give them one continuous dissolve pattern, the vertex shader converts their positions to world space and subtracts the same enemy origin. The body, head and eyes then sample the same noise field. Moving the enemy does not move the pattern across its surface.')
h('Noise, threshold and edge band')
add('The fragment shader generates 3D noise from the eight corners of a grid cell. It smooths the fractional coordinates with t*t*(3-2*t) and interpolates between the corner values. As the progress value rises, fragments below that threshold are discarded. At progress 0 the whole model is visible; at 1 it is completely removed.')
add('position = world position - shared enemy origin<br/>mask = noise3D(position x 5 + fixed offset)<br/>discard if progress &gt;= 1, or if progress &gt; 0 and mask &lt; progress<br/>edge = 1 - smoothstep(0, 0.09, mask - progress)','code')
add('Fragments just above the threshold are coloured orange or yellow. We apply this colour after lighting so that the edge stays bright even in shadow. It does not light nearby objects; it is simply part of the surface colour.')
h('Shadow and gameplay integration')
add('EnemyNode selects the dissolve shader while a defeated monster is disappearing, then restores the previous shader. We use the same discard rule in the shadow pass with u_DepthOnly enabled. This removes the corresponding parts of the shadow as well as the visible model.')
add('DissolveState controls the timer using frame delta time. Pausing the game freezes the timer. The HP bar is hidden when the enemy dies, and the last idle pose is held while the model dissolves. A new spawn resets the timer, hit flash and visual offset. We added tests for these transitions.')


# 7. Animation and interaction.
page('6. Continuous animation and interaction')
h('Continuous idle animation')
add('We added idle animation to every enemy type so that the scene keeps moving when the player is not pressing a button. EnemyNode::_Process updates the model using a sine function and elapsed time. Each monster starts with a slightly different phase so they do not all move together.')
table(['Type','Loop','Transformation'],[
 ['Slime','1.2 s','Squash/stretch: height changes by up to 10%; width/depth by up to 6%. Vertical compensation keeps the lower surface grounded.'],
 ['Goblin','1.7 s','Gentle side-to-side roll of up to 2.5 degrees with a small vertical bob.'],
 ['Golem','2.4 s','Slow breathing through a height variation of up to 2.5%, with grounding compensation.'],
],[70,65,348])
add('wave = sin(2 x pi x time / period)<br/>slime height = 1 - 0.10 x wave<br/>vertical compensation = 0.5 x (height - 1)','code')
add('The idle loops are made by changing the model transforms in code. They run during exploration and while the player waits in battle. Because they affect only the model subtree, the encounter position, HP bar and attack offset stay separate. When the enemy dies, its idle animation stops for the dissolve effect.')
h('Action feedback and camera')
add('For an attack, the character makes a short lunge toward the opponent. A skill uses a longer lunge. Defense adds a small dip, while damage causes a red flash and a shake. Cube particles appear on a hit. We block new actions until this feedback finishes so that several actions cannot overlap.')
add('The camera follows the player and rotates with Q and E. In battle, it looks toward the midpoint between the fighters. We smooth its position and target with blend = 1 - exp(-6 x dt), which avoids sudden jumps and keeps the transition consistent at different frame rates.')
h('Pause and input')
add('Pressing P pauses movement, combat timers, idle animation and dissolve. It also disables action buttons. We clear held movement keys when pausing or resuming to avoid a stuck direction. The HUD shows health, mana, the current turn, battle messages, kills and enemies remaining. Button colours and labels show which actions are available.')

# 8. Tests and reproducibility.
page('7. Testing and running the project')
add('We ran run-game.cmd --check-gl on 19 November 2026 using the submission build. The Debug x64 build completed, and all five test programs passed. The compiler still reported an APIENTRY macro warning, but there were no build errors.')
table(['Test executable','Verified behaviour','Result'],[
 ['BattleTests','Defense MP recovery/cap, turn blocking, skill cost and state preservation.','Passed'],
 ['SpawnLayoutTests','Bounds, safe centre and pairwise spacing across 10,000 seeds.','Passed'],
 ['HudStateTests','Scaled hit testing, button gaps, pause, enemy turn, upgrades and restart.','Passed'],
 ['EnemyNodeTests','Scene hierarchy, all idle types, combat-offset isolation, pause, death, dissolve and respawn.','Passed'],
 ['DissolveShaderTests','Real-driver compile/link, decreasing coverage, ember edge and matching colour/depth masks.','Passed'],
],[117,310,56])
h('Measured dissolve coverage')
add('The GPU test opens a hidden OpenGL 3.3 window and draws a test surface into a 128 x 128 framebuffer. We counted 10,404 visible pixels at progress 0, 7,939 at 0.35, 1,730 at 0.65 and none at 1. The colour and shadow-depth passes removed the same pixels. These are coverage measurements for the test surface, not frame-rate results.')
h('Build and execution')
add('The project is built on Windows with Visual Studio, the v145 C++ toolset and a Windows SDK. Running run-game.cmd builds Debug x64 and starts the game from the repository folder. It also places the FreeGLUT DLL beside the executable. The shaders and textures must stay in their expected folders.')
add('run-game.cmd --check      CPU-side regression suite<br/>run-game.cmd --check-gl   CPU checks plus real-driver GLSL test','code')
add('The automated tests check individual systems. The repeatable benchmark results are given in Section 10. The Debug x64 regression tests passed, and we also built and benchmarked Release x64 on 20 November. The actual lab computer still needs a separate check. Before the defence, we will play through an upgrade, a floor change and a restart on that computer.')

# 9. Conclusions and evidence.
page('8. Conclusion')
h('Results and team collaboration')
add('We completed a playable prototype with exploration, turn-based combat and several graphics effects. Our team, Aibyn Samat, Diyar Sekezhanov and Dias Zhakin, worked on the gameplay, scene nodes, shaders, interface and animation. We used Git to share changes and checked the existing work before adding new features.')
add('One useful improvement was moving enemies and the HUD out of the main file. EnemyNode now keeps the monster data, model and animation together, while HudNode handles the interface. This made changes easier to follow and allowed us to test these parts separately.')
h('Limitations and future work')
add('The game currently reuses one arena and increases enemy strength on later floors. It does not save progress between launches, and there is no final boss. The block animations are simple, but they give the enemies visible movement without requiring external animation files.')
add('If we continue the project, we would first improve balance, add more enemy behaviours and save the progression between sessions. We would also profile the renderer in more detail and test longer runs. For this version, our focus was getting the rendering, animation and battle loop to work together.')
h('AI assistance')
add('We used ChatGPT through Codex to help draft and revise gameplay and node code, the dissolve shader, idle animations, tests, procedural texture generation and this report (OpenAI, 2026). These were substantial assisted contributions. The team is responsible for reviewing the submitted code and understanding the shader equations; the tool does not replace the live explanation and modification required at the defence.')

page('9. Feature checklist')
add('Our free-choice project is Monster Clash. No numbered topic was assigned to us, and Section 6 is absent from the supplied requirements. We therefore use the common technical requirements and our implemented game features as the checklist.')
table(['Requirement','Status and evidence'],[
 ['C++ / OpenGL 3.3+ core','Done. A core 3.3 context is requested in GameApp; GLAD loads the API.'],
 ['1280 x 720 real-time window','Done. Default size is 1280 x 720. FPS and low-quality fallback are documented in Section 10.'],
 ['Perspective and orbit camera','Done. Resize-safe aspect ratio; Q/E rotate the follow camera.'],
 ['Non-flat vertex/fragment pair','Done. Directional lighting, PCF shadows, rim lighting and procedural dissolve.'],
 ['Depth and back-face culling','Done. Outward CCW triangle winding; GL_BACK culling. The 2D HUD explicitly disables both depth and culling.'],
 ['Help, pause and effect switches','Done. H/F1 help, P pause, R rim, T dissolve, B shadows; on-screen toggle states.'],
 ['Adjustable light','Done. J/L rotate the directional light and its shadow camera.'],
 ['Console diagnostics','Done. Texture, shader compile/link and framebuffer failures print errors.'],
 ['Build and external shaders','Done. Visual Studio project and CLI launcher; world, shadow, rim and dissolve GLSL in files.'],
 ['Start, goal, fail and replay','Done. Explore, clear floors, lose, restart. Repeated floors need no process restart.'],
 ['Visible technique in first 15 s','Done. A six-second opening preview shows rim, shadows and looping dissolve; it then enters play. F2 reopens the preview.'],
 ['Names, topic and status','Done. Team names, Monster Clash, floor/kills, effect states and FPS are on screen.'],
 ['Licensed active assets','Done. Procedural cube models and original CC0 textures. Older unused images are excluded from the source ZIP.'],
 ['Continuous animation','Done. Separate slime, goblin and golem loops; P freezes them.'],
 ['Report, source and video','Done. Report and source are supplied; the YouTube demonstration link is included in Section 1.' if META.get('video_url') else 'Partial. Report and source are supplied; the public demonstration video has not yet been uploaded.'],
],[145,338])

page('10. Controls and measured performance')
table(['Keys','Action'],[
 ['WASD / Q, E','Move / orbit the camera.'],['1, 2, 3 / left click','Battle actions or upgrade selection.'],
 ['P / H or F1','Pause all simulation and animation / show help.'],['R / T / B','Toggle player rim / enemy dissolve / shadow mapping.'],
 ['J, L / G','Rotate sun / low-quality preset (skips shadow pass) or restore full quality. B can override shadows.'],
 ['F2 / Space','Repeat shader preview or return to play / restart after defeat.'],
],[110,373])
h('Reproducing the demonstration view')
add('Start the game and watch the opening preview, or press F2 during a run. The middle goblin dissolves once per three-second cycle. Press T during that cycle to compare it with an intact model, R to compare rim lighting, and B to compare shadows. Press P halfway through the dissolve to inspect the edge. J/L rotate the sun. F2 resets the run when returning to play.')
h('Frame-rate measurements')
import csv
rows=list(csv.reader((ROOT/'output/benchmark-release-2026-11-20.csv').open()))
gpu=rows[0][1]; driver=','.join(rows[1][1:])
add(f'We ran the Release x64 build on 20 November 2026 at 1280 x 720 using {escape(gpu)}; OpenGL driver {escape(driver)}. The scene, camera and half-dissolved monster were fixed for every comparison. Each mode used 12 warm-up frames followed by 90 measured frames.')
table(['Mode','Average FPS','Frame time'],[[r[0],f'{float(r[5]):.1f}',f'{float(r[4])/int(r[3]):.2f} ms'] for r in rows[3:]],[260,110,113])
add('These CPU-observed rates include drawing, buffer swaps and glFinish, rather than isolated GPU time. Full quality reached 141.1 FPS; the other modes were close to 144 FPS, above the 30 FPS target. Ordinary gameplay follows a separate 16 ms GLUT timer, so its update rate is approximately 60 FPS. We did not verify an explicit V-Sync setting. The earlier Debug results are retained in output/benchmark.csv; this Release run is saved in output/benchmark-release-2026-11-20.csv. The lab PC still needs a check.')

page('11. Graphics pipeline and build capture')
add('Dissolve takes the indexed block meshes, model/view/projection matrices, a shared enemy origin and a progress value. The same vertex transformation and fragment discard mask run in two passes: light-space depth and camera-space colour. The result feeds the final lit scene; the HUD is drawn afterwards.')
class Pipeline(Flowable):
    def __init__(self): Flowable.__init__(self); self.width=W; self.height=174
    def draw(self):
        c=self.canv; c.setStrokeColor(colors.black); c.setFillColor(colors.white)
        boxes=[(0,126,483,30,'Inputs: meshes / transforms / enemy origin / progress'),
               (0,68,229,34,'Depth pass: dissolve VS + FS'),(254,68,229,34,'Colour pass: dissolve VS + FS'),
               (0,10,229,30,'Output: masked sun depth texture'),(254,10,229,30,'Output: lit colour + bright edge')]
        for x,y,w,hh,label in boxes:
            c.rect(x,y,w,hh,fill=0);c.setFillColor(colors.black);c.setFont('Body',10);c.drawCentredString(x+w/2,y+hh/2-3,label)
        for x in [114,368]: c.line(x,126,x,102);c.line(x,68,x,40)
        c.line(229,25,254,25);c.setFont('Body',8);c.drawCentredString(241,48,'sample')
story.append(Pipeline())
add('Figure 2. The dissolve mask is shared by the depth and colour passes. The colour pass samples the depth texture to compute visibility.','small')
from PIL import Image as PILImage
capture=HERE/'build_capture.png'
if (ROOT/'output/benchmark-0.ppm').exists(): PILImage.open(ROOT/'output/benchmark-0.ppm').convert('RGB').save(capture)
story.append(Image(str(capture),width=W,height=W*720/1280))
add('Figure 3. Screenshot from our submission build at 1280 x 720. The goblin is halfway through its dissolve.','small')
add('The world lighting and dissolve formulas are shown in Sections 4 and 5. The rim calculation uses the actual camera position, and its shader is bypassed for the depth pass. This keeps the player shadow independent of its rim colour. Zero-scale pooled particles, hidden model variants and hidden bars skip rendering, which also avoids inverting singular normal matrices.')

page('12. Contributions, assets and references')
h('Individual contributions')
table(['Team member','Module ownership and contribution'],[
 ['Aibyn Samat','Gameplay and enemy modules: battle loop, enemy/HUD refactoring, procedural dissolve, idle animation, regression tests and report preparation.'],
 ['Diyar Sekezhanov','Player and arena modules: PlayerNode, arena assembly, player shader experiments and rim-light shader integration.'],
 ['Dias Zhakin','Shadow and feedback modules: sun depth pass, PCF shadows, HUD presentation, particles and hit/camera feedback.'],
],[120,363])
h('Asset sources and licenses')
add('All active character and enemy meshes are assembled from cubes in our source. We generated grass, road and wall textures with the included Assets/generated/generate_textures.py. They are distributed under CC0 1.0 (Creative Commons, n.d.). The generator and license are in the repository at the asset URL below. Older images with unconfirmed provenance and the disabled external OBJ are not used and are excluded from the source ZIP.')
add('Texture source and generator: https://github.com/MasterDiyar/FinalProjectGraphics/tree/feature/battle/Assets/generated','small')
h('References')
refs=[
 'Creative Commons. (n.d.). <i>CC0 1.0 Universal</i>. https://creativecommons.org/publicdomain/zero/1.0/',
 'G-Truc Creation. (n.d.). <i>OpenGL Mathematics (GLM)</i> [Computer software]. GitHub. https://github.com/g-truc/glm',
 'Herberth, D. (2021). <i>glad</i> (Version 0.1.36) [Computer software]. GitHub. https://github.com/Dav1dde/glad/tree/v0.1.36',
 'Monster Clash team. (2026). <i>Monster Clash</i> [Source code and procedural assets]. GitHub. https://github.com/MasterDiyar/FinalProjectGraphics/tree/feature/battle',
 'OpenAI. (2026). <i>ChatGPT (Codex)</i> [Large language model]. https://chatgpt.com/',
 'Olszta, P. W., &amp; FreeGLUT contributors. (n.d.). <i>FreeGLUT</i> [Computer software]. https://freeglut.sourceforge.net/',
 'Segal, M., &amp; Akeley, K. (2010). <i>The OpenGL graphics system: A specification (Version 3.3, core profile)</i>. Khronos Group. https://registry.khronos.org/OpenGL/specs/gl/glspec33.core.pdf',
 'stb contributors. (n.d.). <i>stb</i> [Computer software]. GitHub. https://github.com/nothings/stb',
]
for ref in refs: add(ref,'small')


pdf=HERE/'Monster_Clash_Project_Report.pdf'
doc=SimpleDocTemplate(str(pdf),pagesize=(595.2756,841.8898),leftMargin=56,rightMargin=56,topMargin=65,bottomMargin=62,
    title='Monster Clash - Final Project Report',author=', '.join(META['students']))
doc.build(story,onFirstPage=footer,onLaterPages=footer)
print(pdf)
