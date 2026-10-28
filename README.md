# Monster Clash — C++ / OpenGL final project

A small 3D roguelike with a colorful, blocky visual style and Pokemon-inspired
turn-based encounters. Working title: Monster Clash.

## Current prototype

- Explore a single arena with a following camera and bounded movement.
- Encounter three randomly positioned enemies: Slime, Goblin and Golem.
- Choose attack, skill (3 MP), or defense; the enemy responds after a short delay.
- Win a battle to choose an attack, health, or mana upgrade.
- Clear all enemies to advance to a harder floor.
- Restart after defeat with a permanent +1 attack bonus for this session.
- A tiled arena with crossing paths, boundary walls and corner pillars.
- A blocky hero and monster faces built from scene-graph children, with simple
  directional diffuse lighting. No downloaded art assets are required.
- OBJ loading and texture support; the built-in block hero is used when
  `assets/hero.obj` is absent.

## Build and run (Windows)

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

## Controls

| State | Controls |
| --- | --- |
| Exploration | WASD (also with Shift/Caps Lock) |
| Battle | 1 attack, 2 skill, 3 defend |
| Upgrade | 1 +2 attack, 2 +8 max HP and heal, 3 +4 max MP and refill |
| Defeat | Space to restart |

The on-screen HUD shows HP/MP bars, floor, kills, attack, current turn and battle
messages. Click an action button or use its keyboard shortcut. Battle buttons
dim during the enemy turn; the skill button also dims below 3 MP. Upgrade and
restart buttons are clickable too. The HUD and mouse coordinates scale together
when the window is resized. Its small built-in bitmap font uses OpenGL triangles
and adds no dependency.

## Next milestones

1. Distinct silhouettes for each enemy type and additional arena details.
2. Attack/hit animations and a distinct battle camera.
3. Enemy spacing, balance and a final encounter with a clear win screen.
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

