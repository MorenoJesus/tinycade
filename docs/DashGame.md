# Dash Game

## Overview

`DashGame.h` adds a Geometry Dash-inspired ESP32 game module that fits the shared `IGame` contract used by the launcher. It is header-only, self-contained, and designed for the 135x240 TFT layout used by the Tinycade project.

## Controls

- `BTN2` jumps during gameplay.
- A short press of both buttons retries from game over.
- On the intro screen, either button starts the run.

## Gameplay

- The player auto-runs from left to right.
- Obstacles scroll in from the right and are respawned ahead of the player.
- Score increases as obstacles are passed.
- The game ramps speed gradually so later runs feel tighter without becoming impossible.
- After a retry, the game uses a short grace window before obstacle collisions can kill the player.

## Visual Style

- Dark neon background with a grid and floating dots.
- Bright cube player with glow treatment.
- Spikes and blocks are rendered in a Geometry Dash-like style instead of the Runner or Dino look.

## State Flow

- `Intro`
- `Running`
- `GameOver`

## Implementation Notes

- The module uses the shared `GameContext` and `InputState` from `GameTypes.h`.
- The module is now registered in the launcher game list through [Tinycade.ino](../Tinycade.ino).
- The file is header-only and follows the same plug-in pattern used by the other Tinycade game modules.

## Files Created

- [Games/DashGame.h](../Games/DashGame.h)
