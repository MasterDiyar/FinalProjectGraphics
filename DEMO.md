# Prototype demo — Friday, October 30, 2026

## Before the presentation

- Build Debug x64; run from the repository root.
- Check WASD, mouse clicks and number keys on the presentation computer.
- Resize the window and confirm that text and buttons remain usable.
- Press P during exploration and during an enemy turn: movement, attacks and
  animations should freeze; P resumes without spending extra turns.
- Play one battle through to an upgrade; check the next encounter.
- Try a skill below 3 MP and confirm it does not spend a turn.
- Check attack/skill lunges, defense motion and red hit flashes on both actors.
- Check that a final hit animates before the upgrade/defeat screen and that
  actors return to their original positions after feedback.
- Check defeat and Space/click restart before the live demo.

## A short demo (about 3 minutes)

1. Explain the idea: a C++/OpenGL 3D roguelike combining blocky visuals with
   Pokemon-inspired turn-based combat.
2. Walk around the arena. Point out the following camera, tiled paths,
   boundary walls, hero model and health/mana HUD.
3. Touch a monster. Show the enemy HP, turn indicator and combat message.
4. Attack, defend, then use a skill. Explain that skills cost 3 MP and the enemy
   responds after the player chooses an action.
5. Win and choose an upgrade. Explain that clearing the arena starts a harder
   floor, while defeat restarts the run with a session-only attack bonus.
6. State the remaining work: enemy variety, animation, balance, a final
   encounter and saving progression between launches.

## Technical points for questions

- C++20, OpenGL/GLAD, FreeGLUT for window/input, GLM for matrices, stb_image
  for textures; the included GLFW library is currently unused.
- Scene graph: models are assemblies of transformed cube children.
- Lighting: ambient plus directional diffuse shading; flat face normals come
  from fragment derivatives of world position. There are no cast shadows yet.
- HUD: bitmap glyphs and rectangles drawn as OpenGL triangles, without a UI
  framework; mouse and keyboard share the same action logic.
- Game state: exploration, battle, upgrade and defeat.

Present this as a working prototype. Meta progression currently survives only
within the running application. Build/visual checks do not replace a complete
manual gameplay rehearsal on the machine used for the presentation.
