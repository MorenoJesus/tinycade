#include <SPI.h>
#include <TFT_eSPI.h>

#include "GameTypes.h"
#include "Launcher.h"
#include "Games/BreakoutGame.h"
#include "Games/DashGame.h"
#include "Games/DinoGame.h"
#include "Games/FlappyGame.h"
#include "Games/PunchGame.h"
#include "Games/ReactionGame.h"
#include "Games/RunnerGame.h"
#include "Games/SnakeGame.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

BreakoutGame breakoutGame;
DashGame dashGame;
DinoGame dinoGame;
FlappyGame flappyGame;
PunchGame punchGame;
ReactionGame reactionGame;
RunnerGame runnerGame;
SnakeGame snakeGame;

IGame *games[] = {
    &breakoutGame,
    &dashGame,
    &dinoGame,
    &flappyGame,
    &punchGame,
    &reactionGame,
    &runnerGame,
    &snakeGame,
};

Launcher launcher(games, sizeof(games) / sizeof(games[0]));

namespace {
bool gLastFlapDown = false;
bool gLastActionDown = false;
bool gPendingFlapPressed = false;
bool gPendingActionPressed = false;
bool gPendingFlapReleased = false;
bool gPendingActionReleased = false;
bool gCurrentFlapDown = false;
bool gCurrentActionDown = false;
uint32_t gLastFrameMs = 0;
uint32_t gBothHoldStartMs = 0;

bool readPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void sampleInput(uint32_t nowMs) {
  bool flapDown = readPressed(BTN_FLAP);
  bool actionDown = readPressed(BTN_ACTION);

  if (flapDown && !gLastFlapDown) {
    gPendingFlapPressed = true;
  }
  if (actionDown && !gLastActionDown) {
    gPendingActionPressed = true;
  }
  if (!flapDown && gLastFlapDown) {
    gPendingFlapReleased = true;
  }
  if (!actionDown && gLastActionDown) {
    gPendingActionReleased = true;
  }

  gCurrentFlapDown = flapDown;
  gCurrentActionDown = actionDown;

  if (gCurrentFlapDown && gCurrentActionDown) {
    if (gBothHoldStartMs == 0) {
      gBothHoldStartMs = nowMs;
    }
  } else {
    gBothHoldStartMs = 0;
  }

  gLastFlapDown = flapDown;
  gLastActionDown = actionDown;
}

InputState pollInput(uint32_t nowMs) {
  InputState input;
  input.flapDown = gCurrentFlapDown;
  input.actionDown = gCurrentActionDown;
  input.flapPressed = gPendingFlapPressed;
  input.actionPressed = gPendingActionPressed;
  input.flapReleased = gPendingFlapReleased;
  input.actionReleased = gPendingActionReleased;
  input.bothHeldMs = (input.flapDown && input.actionDown && gBothHoldStartMs > 0)
                         ? nowMs - gBothHoldStartMs
                         : 0;

  gPendingFlapPressed = false;
  gPendingActionPressed = false;
  gPendingFlapReleased = false;
  gPendingActionReleased = false;
  return input;
}
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_FLAP, INPUT);
  pinMode(BTN_ACTION, INPUT_PULLUP);

  pinMode(DISPLAY_BACKLIGHT_PIN, OUTPUT);
  digitalWrite(DISPLAY_BACKLIGHT_PIN, HIGH);

  tft.init();
  tft.setRotation(0);
  sprite.setColorDepth(16);
  sprite.createSprite(SCREEN_W, SCREEN_H);
  sprite.setSwapBytes(true);

  randomSeed(micros());
  gLastFrameMs = millis();
  launcher.begin();
}

void loop() {
  uint32_t nowMs = millis();
  sampleInput(nowMs);

  if (nowMs - gLastFrameMs < FRAME_DELAY_MS) {
    return;
  }

  uint32_t deltaMs = nowMs - gLastFrameMs;
  gLastFrameMs = nowMs;

  InputState input = pollInput(nowMs);
  GameContext ctx = {&tft, &sprite, nowMs, deltaMs, false};

  launcher.tick(ctx, input);
  sprite.pushSprite(0, 0);
}
