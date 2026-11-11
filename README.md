# Monster Clash — C++ / OpenGL final project

A small 3D roguelike with a colorful, blocky visual style and Pokemon-inspired
turn-based encounters. Working title: Monster Clash.

## Current prototype

- Explore a single arena with a following camera and bounded movement.
- Encounters smoothly switch to a side camera centered between both fighters;
  exploration framing returns after combat. Pause also freezes camera motion.
- Encounter three randomly positioned enemies: Slime, Goblin and Golem.
- Spawns use shuffled zones with random offsets: at least 4.5 units between
  enemies, a safe central starting area, and no overlapping encounter zones.
- Enemy silhouettes match their type: a squat slime, a goblin with wide ears,
  and a broad stone golem. Models switch with each new spawn and retain combat
  movement and hit flashes through their shared scene-graph parent.
- Choose attack, skill (3 MP), or defense; the enemy responds after a short delay.
- Attacks lunge toward the opponent, skills use a longer lunge, defense dips the
  character, and hits briefly flash red with a small shake. Visual offsets do
  not change encounter positions. Actions wait for feedback to finish, including
  the final blow before an upgrade or defeat screen.
- Win a battle to choose an attack, health, or mana upgrade.
- Clear all enemies to advance to a harder floor.
- Restart after defeat with a permanent +1 attack bonus for this session.
- A tiled arena with crossing paths, boundary walls and corner pillars.
- A blocky hero and monster faces built from scene-graph children, with simple
  directional diffuse lighting. No downloaded art assets are required.
- OBJ loading and texture support; the built-in block hero is used when
  `assets/hero.obj` is absent.

## Build and run (Windows)

Double-click `run-game.cmd`, or run it from a terminal. It locates Visual Studio,
builds Debug x64, and launches from the repository root so assets resolve.
Visual Studio with Desktop development with C++, the v145 toolset and Windows
SDK must already be installed; the launcher does not install dependencies.

Before a presentation, build and run all regression checks without opening a
game window:

```bat
run-game.cmd --check
```

The command stops on build/test failure. Close any running game before building.

Open `FinalProjectGraphics.slnx` in Visual Studio with Desktop development with C++,
Windows SDK and the v145 toolset. Select **Debug | x64** and build.
Alternatively, in a Visual Studio Developer PowerShell:

```powershell
msbuild OpenJL.vcxproj /p:Configuration=Debug /p:Platform=x64
.\x64\Debug\OpenJL.exe
```

Run from the repository root so relative asset paths resolve. The bundled
FreeGLUT runtime is copied next to the executable automatically in Debug x64.
The bundled import library requests `freeglut.dll`, so the build copies the
provided `freeglutd.dll` under that name. Bundled libraries are intended for x64.
Release has not been validated yet; it needs its matching FreeGLUT runtime DLL.

Rendering uses OpenGL, GLAD, GLM, FreeGLUT and stb_image. GLFW is bundled but is
not used by the current window/input implementation. No engine is required.

## Code structure

Enemy defeat now uses a dedicated GLSL dissolve effect with an ember edge and
matching shadow removal. See [shader explanation and demo steps](shaders/README.md).
Run `run-game.cmd --check-gl` for regression checks plus real-driver GLSL tests.

- `Main.cpp`: application entry point only; calls `runGame`.
- `Game/GameApp.cpp`: window callbacks, scene assembly, combat orchestration,
  camera and renderer initialization. Owns the run state.
- `Game/EnemyNode.h/.cpp`: a `Node` that owns the fighter, model variants,
  animation anchor and HP bar. `spawn` selects its type and floor scaling;
  `_Process` updates visibility and the bar, including final-hit feedback.
- `Game/HudNode.h/.cpp`: a `Node` that draws the HUD as the last scene pass.
  Receives a `HudState` value snapshot; it does not mutate combat state.
- `Game/HudState.h`: button availability and hit testing on the shared 800x600
  canvas. The mouse callback forwards the returned action to the same game
  logic used by keyboard input. `Game/Hud.h` remains the text/rectangle renderer.

The root scene owns the arena, player, player HP bar, three `EnemyNode` children
and finally `HudNode`. Each mob owns its body/model subtree and its own HP bar;
the scene traverses their updates and renders normally. Keep `HudNode` last so
screen-space drawing follows all world geometry.

`run-game.cmd --check` covers combat, spawn spacing, HUD input/state and enemy
node lifecycle/hierarchy. The node test exercises CPU-side updates without a
graphics context; it does not verify the rendered image.

## Controls

| State | Controls |
| --- | --- |
| Exploration | WASD (also with Shift/Caps Lock) |
| Battle | 1 attack, 2 skill, 3 defend (restore up to 2 MP) |
| Upgrade | 1 +2 attack, 2 +8 max HP and heal, 3 +4 max MP and refill |
| Defeat | Space to restart |
| Any state | P to pause/resume |

Defense halves incoming damage and restores up to 2 MP without exceeding the
maximum. It spends a turn, so the enemy still responds. A skill with insufficient
MP leaves the turn, resources and defense state unchanged.

Pause freezes movement, combat timers and feedback animations. Actions and
upgrade choices are disabled until resumed. Movement keys are cleared when
toggling pause so a previously held direction does not remain active.

The on-screen HUD shows HP/MP bars, floor, kills, attack, current turn and battle
messages, plus the number of surviving enemies. Exploration shows a floor-clear
progress bar, and the last victory explains that choosing an upgrade advances
to the next floor. Counts come from living enemies and reset with each floor/run.
Click an action button or use its keyboard shortcut. Battle buttons
dim during the enemy turn; the skill button also dims below 3 MP. Upgrade and
restart buttons are clickable too. The HUD and mouse coordinates scale together
when the window is resized. Its small built-in bitmap font uses OpenGL triangles
and adds no dependency.

## Spawn layout checks

Spawn layout regression check (Visual Studio Developer Command Prompt):

```bat
cl /nologo /EHsc /std:c++20 Tests\SpawnLayoutTests.cpp /Fo:x64\SpawnLayoutTests.obj /Fe:x64\SpawnLayoutTests.exe
x64\SpawnLayoutTests.exe
```

The check covers arena bounds, center clearance and pairwise spacing over
10,000 random seeds. Build the game first so the `x64` directory exists.

## Battle checks

Battle regression check (Visual Studio Developer Command Prompt):

```bat
cl /nologo /EHsc /std:c++20 Tests\BattleTests.cpp /Fo:x64\BattleTests.obj /Fe:x64\BattleTests.exe
x64\BattleTests.exe
```

## Next milestones

1. Additional arena details and enemy behavior variety.
2. Further combat animation polish.
3. Balance and a final encounter with a clear win screen.
4. Save meta progression between launches; prepare a demo and presentation.

Suggested split for three teammates (assign names together):

- Graphics: lighting, arena, models, camera and animations.
- Gameplay: combat, enemy behavior, upgrades, balance and saves.
- Interface/integration: HUD, input, build instructions and demo preparation.

Keep each change small enough to build and demonstrate. Before a demo, check
movement along edges and diagonals, window resizing, each battle action,
insufficient MP, all upgrades, floor transitions, defeat and restart.

## Scope and references

The required platform is C++ with OpenGL. The concept combines Roblox-inspired
blocky visuals, Pokemon-inspired turn-based choices and roguelike restarts with
partial progression. These are inspirations; original art and naming can be
chosen by the team.

[Original concept discussion](https://claude.ai/share/de4ce892-9249-4411-8af0-f38e7303b0fb)

## Friday prototype demonstration

See [DEMO.md](DEMO.md) for a short presentation sequence and pre-demo checks.

