# Tinycade

Tinycade is a pocket-sized retro arcade for ESP32: a tiny launcher, a TFT screen, two buttons, and a growing stack of fast little games.

## Insert Coin

Current game list:

- Breakout
- Dash
- Dino
- Flappy
- Punch
- Reaction
- Runner
- Snake

## Cabinet Rules

- `BTN1`: Move to the next game in the launcher
- `BTN2`: Launch the selected game
- In most action games, the in-game buttons are intentionally reversed from the menu layout
- Hold both buttons for about `1.5s` in-game to return to the launcher
- On modern game-over screens, retry now requires a short press of both buttons so button mashing does not throw you straight back into danger

## Built For

- ESP32
- `135x240` TFT display
- `TFT_eSPI`
- Arduino IDE

## Start Game

1. Open [Tinycade.ino](./Tinycade.ino).
2. Verify your `TFT_eSPI` setup matches your display hardware.
3. Select the right ESP32 board and port.
4. Compile and upload.

## Docs

Full project docs live in [docs/README.md](./docs/README.md).

Highlights:

- [Architecture](./docs/Architecture.md)
- [Setup And Upload](./docs/SetupAndUpload.md)
- [Game Catalog](./docs/GameCatalog.md)
- [File Reference](./docs/FileReference.md)

## Tinycade Mood

Think neon arcade floors, cheap speakers, glowing marquees, and that one machine in the corner that somehow always has the best game on it.
