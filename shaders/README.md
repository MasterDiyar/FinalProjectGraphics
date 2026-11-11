# Enemy dissolve — individual shader effect

This effect is separate from Diyar's player rim lighting and IshShlange's sun
shadow mapping. Files: `enemy_dissolve.vert` and `enemy_dissolve.frag` (GLSL 330).
It is a procedural dissolve with a bright ember edge, not a blur/bloom pass.

## How to show it

Defeat any monster. Its final-hit animation finishes first, then its surface
breaks apart for 0.9 seconds before the upgrade screen appears. The HP bar hides
on death. Pause with P to freeze the effect; a new floor/run resets it.

## How it works

1. The vertex shader transforms vertices to world space and subtracts a shared
   mob origin. Head, arms, eyes and body therefore sample one continuous field,
   even though they are separate cube meshes. Moving a mob does not slide the
   noise pattern across its surface.
2. The fragment shader builds smooth 3D value noise from the eight hashed corners
   of a grid cell, interpolated with `t*t*(3-2*t)`. No noise texture is required.
3. `u_DissolveProgress` increases from 0 to 1. Fragments whose noise falls below
   this threshold are removed with `discard`; surviving fragments still write
   depth. Explicit endpoints guarantee a whole model at 0 and no model at 1.
4. A narrow band above the threshold blends toward orange/yellow. This band is
   emissive in the sense that it is applied after diffuse lighting, so it remains
   bright in shadow. It does not cast light onto other objects.
5. The same program runs in the shadow pass with `u_DepthOnly=true`: it discards
   exactly the same fragments and writes only the surviving depth. The color
   pass keeps the team's ambient/diffuse and shadow-map calculations.

`EnemyNode` owns the death clock and switches programs only for dissolving mobs.
After rendering, it restores the caller's shader program. Player rim lighting,
living mobs and HUD retain their existing rendering paths. `GameApp` waits for
completion before awarding the upgrade. `MeshNode` skips geometry-free anchors
but continues rendering their children.

## Main uniforms

| Uniform | Role |
| --- | --- |
| `u_MVP`, `u_Model` | Clip and world transforms for each child mesh |
| `u_DissolveOrigin` | Shared world origin of the mob |
| `u_DissolveProgress` | Dissolve threshold, 0..1 |
| `u_DepthOnly` | Select shadow-depth output |
| `u_Color`, `u_Tex`, `u_UseTex`, `u_HitFlash` | Existing material/feedback inputs |
| `u_SunDir`, `u_LightVP`, `u_ShadowMap` | Existing sun and shadow inputs |

## Verification

`run-game.cmd --check` exercises the death clock, pause, final-hit wait, respawn,
and the existing gameplay/input checks.

`run-game.cmd --check-gl` also opens a hidden OpenGL 3.3 window and renders to an
offscreen framebuffer using the actual `.vert`/`.frag` files. It checks shader
compile/link, strictly decreasing coverage at 0/0.35/0.65/1, a visible ember edge,
identical color/depth discard masks and OpenGL errors. This needs a working
graphics driver and desktop session. It does not replace an interactive battle
check on the computer used for the presentation.
