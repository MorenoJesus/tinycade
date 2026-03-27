# Game Catalog

## Menu Controls

- `BTN1`: Next game
- `BTN2`: Launch game

## Global In-Game Exit

Hold both buttons for roughly `1.5 seconds` to return to the menu.

## Game-Over Retry

Most current game-over screens require a short press of both buttons to retry. This helps prevent accidental restarts while the player is still mashing inputs after a loss.

## Games

### Breakout

- File: [BreakoutGame.h](../Games/BreakoutGame.h)
- Genre: Brick breaker
- Controls:
  `BTN2` moves left
  `BTN1` moves right

### Dash

- File: [DashGame.h](../Games/DashGame.h)
- Genre: Geometry Dash-inspired rhythm platform runner
- Controls:
  `BTN2` jumps and starts
  both buttons retry on game over
- Notes:
  includes a short retry grace window before obstacle collisions become active

### Dino

- File: [DinoGame.h](../Games/DinoGame.h)
- Genre: Chrome T-Rex style endless runner
- Controls:
  `BTN2` jumps
  `BTN1` ducks / boosts
- Notes:
  includes a short retry grace window before obstacle collisions become active

### Flappy

- File: [FlappyGame.h](../Games/FlappyGame.h)
- Genre: Flappy Bird clone
- Controls:
  `BTN2` flaps / starts
  both buttons retry on game over

### Punch

- File: [PunchGame.h](../Games/PunchGame.h)
- Genre: Reaction fighter
- Controls:
  `BTN2` attacks left
  `BTN1` attacks right
- Notes:
  includes a short retry grace window before enemies begin spawning again

### Reaction

- File: [ReactionGame.h](../Games/ReactionGame.h)
- Genre: Reaction timing game
- Controls:
  `BTN2` reacts / retries
  `BTN1` returns to title or restart flow depending on state

### Runner

- File: [RunnerGame.h](../Games/RunnerGame.h)
- Genre: Endless runner
- Controls:
  `BTN2` jumps
  `BTN1` ducks / boosts
- Notes:
  includes a short retry grace window before obstacle collisions become active

### Snake

- File: [SnakeGame.h](../Games/SnakeGame.h)
- Genre: Grid snake
- Controls:
  `BTN2` turns left
  `BTN1` turns right
