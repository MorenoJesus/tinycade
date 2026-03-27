#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

static const int SCREEN_W = 135;
static const int SCREEN_H = 240;

static const int BTN_FLAP = 35;
static const int BTN_ACTION = 0;
static const int DISPLAY_BACKLIGHT_PIN = 4;

static const uint32_t FRAME_DELAY_MS = 16;
static const uint32_t EXIT_HOLD_MS = 1500;

enum AppMode {
  MODE_MENU,
  MODE_GAME
};

struct InputState {
  bool flapDown;
  bool actionDown;
  bool flapPressed;
  bool actionPressed;
  bool flapReleased;
  bool actionReleased;
  uint32_t bothHeldMs;
};

struct GameContext {
  TFT_eSPI *tft;
  TFT_eSprite *sprite;
  uint32_t nowMs;
  uint32_t deltaMs;
  bool requestExitToMenu;
};

inline bool shortPressBoth(const InputState &input) {
  return input.flapDown && input.actionDown &&
         (input.flapPressed || input.actionPressed);
}

class IGame {
 public:
  virtual ~IGame() {}

  virtual const char *name() const = 0;
  virtual void init(GameContext &ctx) = 0;
  virtual void handleInput(GameContext &ctx, const InputState &input) = 0;
  virtual void update(GameContext &ctx) = 0;
  virtual void draw(GameContext &ctx) = 0;
  virtual void cleanup(GameContext &ctx) { (void)ctx; }
};
