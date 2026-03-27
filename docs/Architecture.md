# Tinycade Architecture

## Overview

Tinycade is an ESP32 handheld game launcher built around a shared game interface and a single rendering/input loop. The root sketch owns the display, button polling, frame timing, and launcher state. Individual games are isolated modules that plug into the launcher through a common contract.

## Core Files

- [Tinycade.ino](../Tinycade.ino): Arduino sketch entry point. Initializes hardware, polls buttons, builds `GameContext`, and advances the launcher every frame.
- [GameTypes.h](../GameTypes.h): Shared constants, mode enums, input data, render context, and the `IGame` interface.
- [Launcher.h](../Launcher.h): Launcher class declaration.
- [Launcher.cpp](../Launcher.cpp): Menu rendering, menu navigation, active game lifecycle, and the global hold-to-exit overlay.

## Runtime Model

The app runs in one of two modes:

- `MODE_MENU`: Shows the Tinycade menu and lets the player cycle through games.
- `MODE_GAME`: Delegates input, update, and draw calls to the selected game module.

The main loop performs these steps:

1. Read the current button state.
2. Latch fast button events so very short taps are not lost between rendered frames.
3. Track how long both buttons are held together.
4. Build a `GameContext` for the current frame.
5. Call `launcher.tick(ctx, input)`.
6. Push the shared sprite buffer to the TFT.

## Shared Interfaces

### `InputState`

`InputState` contains both held-state and edge-state input:

- `flapDown`
- `actionDown`
- `flapPressed`
- `actionPressed`
- `flapReleased`
- `actionReleased`
- `bothHeldMs`

### `GameContext`

`GameContext` gives every game access to:

- `TFT_eSPI *tft`
- `TFT_eSprite *sprite`
- `nowMs`
- `deltaMs`
- `requestExitToMenu`

### `IGame`

Every game implements:

```cpp
virtual const char *name() const = 0;
virtual void init(GameContext &ctx) = 0;
virtual void handleInput(GameContext &ctx, const InputState &input) = 0;
virtual void update(GameContext &ctx) = 0;
virtual void draw(GameContext &ctx) = 0;
virtual void cleanup(GameContext &ctx);
```

## Display and Performance

- Resolution: `135x240`
- Rendering uses a single shared `TFT_eSprite`
- Frame pacing uses `FRAME_DELAY_MS`
- Backlight pin is `DISPLAY_BACKLIGHT_PIN`

## Input Conventions

### Menu

- `BTN1` / `BTN_FLAP`: Next game
- `BTN2` / `BTN_ACTION`: Launch selected game

### In-Game

Most current games use the reversed in-game mapping requested during development:

- `BTN1`: Former button-2 action
- `BTN2`: Former button-1 action

### Global Exit

Inside any game, holding both buttons for about `1.5s` returns to the menu.

### Retry Behavior

Most modern game-over screens now require a short press of both buttons to restart. This prevents accidental instant restarts while the player is still mashing buttons after a loss.

Fast reaction games such as Dash, Runner, Dino, and Punch also use a short retry grace window before hazards become active again.

## Extending Tinycade

To add a new game:

1. Create a new header-only module in `Games/` that inherits from `IGame`.
2. Include it in [Tinycade.ino](../Tinycade.ino).
3. Instantiate it near the other game objects.
4. Add its pointer to the `games[]` array.
5. Add a matching markdown file in `docs/`.
