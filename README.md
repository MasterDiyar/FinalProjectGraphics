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
- OBJ loading and texture support; the player falls back to a blue cube when
  `assets/hero.obj` is absent. Art assets are not included yet.

## Build and run (Windows)

Open `FinalProjectGraphics.slnx` in Visual Studio with Desktop development with C++,
Windows SDK and the v145 toolset. Select **Debug | x64** and build.
Alternatively, in a Visual Studio Developer PowerShell:

```powershell
msbuild OpenJL.vcxproj /p:Configuration=Debug /p:Platform=x64
.\x64\Debug\OpenJL.exe
```

Run from the repository root so relative asset paths resolve. The bundled
`freeglutd.dll` must be available to the executable (copy it next to the exe if
Windows reports a missing DLL). Bundled libraries are intended for x64.
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

Health bars appear above actors. Stats, actions and battle messages currently
appear in the window title; a proper on-screen HUD is planned.

## Next milestones

1. On-screen HUD: HP/MP, current turn, combat log and clickable action buttons.
2. Blocky character models, arena decoration, normals and simple lighting.
3. Attack/hit animations and a distinct battle camera.
4. Enemy spacing, balance and a final encounter with a clear win screen.
5. Save meta progression between launches; prepare a demo and presentation.

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

