#pragma once

#include "../GameTypes.h"
#include <string.h>

class ReactionGame : public IGame {
 public:
  const char *name() const override { return "Reaction"; }

  void init(GameContext &ctx) override {
    (void)ctx;
    bestReactionMs_ = 0;
    measuredReactionMs_ = 0;
    roundCount_ = 0;
    setState(Splash, 0);
  }

  void handleInput(GameContext &ctx, const InputState &input) override {
    if (state_ == Splash) {
      if (input.flapPressed || input.actionPressed) {
        scheduleRound(ctx.nowMs);
        setState(Ready, ctx.nowMs);
      }
      return;
    }

    if (state_ == Ready) {
      return;
    }

    if (state_ == Waiting) {
      if (input.actionPressed) {
        setState(FalseStart, ctx.nowMs);
      } else if (input.flapPressed) {
        setState(Splash, ctx.nowMs);
      }
      return;
    }

    if (state_ == Go) {
      if (input.actionPressed) {
        measuredReactionMs_ = ctx.nowMs - reactionStartMs_;
        if (bestReactionMs_ == 0 || measuredReactionMs_ < bestReactionMs_) {
          bestReactionMs_ = measuredReactionMs_;
        }
        ++roundCount_;
        setState(Result, ctx.nowMs);
      } else if (input.flapPressed) {
        setState(Splash, ctx.nowMs);
      }
      return;
    }

    if (state_ == Result || state_ == FalseStart) {
      if (input.actionPressed) {
        measuredReactionMs_ = 0;
        scheduleRound(ctx.nowMs);
        setState(Ready, ctx.nowMs);
      } else if (input.flapPressed) {
        setState(Splash, ctx.nowMs);
      }
    }
  }

  void update(GameContext &ctx) override {
    if (state_ == Ready && ctx.nowMs - stateStartedMs_ > 1100) {
      setState(Waiting, ctx.nowMs);
      return;
    }

    if (state_ == Waiting && ctx.nowMs >= nextPromptMs_) {
      reactionStartMs_ = ctx.nowMs;
      setState(Go, ctx.nowMs);
      return;
    }

    if (state_ == Go && measuredReactionMs_ == 0 && ctx.nowMs - reactionStartMs_ > 2500) {
      measuredReactionMs_ = 2500;
      if (bestReactionMs_ == 0 || measuredReactionMs_ < bestReactionMs_) {
        bestReactionMs_ = measuredReactionMs_;
      }
      ++roundCount_;
      setState(Result, ctx.nowMs);
    }
  }

  void draw(GameContext &ctx) override {
    TFT_eSprite &sprite = *ctx.sprite;
    uint16_t bg = TFT_NAVY;
    if (state_ == Go) bg = TFT_GREEN;
    if (state_ == FalseStart) bg = TFT_RED;
    if (state_ == Result) bg = TFT_MAROON;

    sprite.fillScreen(bg);
    sprite.fillCircle(-10, -8, 52, TFT_WHITE);
    sprite.fillCircle(120, 20, 34, TFT_WHITE);
    sprite.fillCircle(102, 210, 46, TFT_WHITE);

    if (state_ == Splash) {
      card(sprite, TFT_DARKCYAN);
      title(sprite, "Reaction", 62, 2, TFT_DARKCYAN);
      title(sprite, "Test", 88, 2, TFT_DARKCYAN);
      body(sprite, 126, "BTN2 react");
      body(sprite, 140, "BTN1 restart");
      body(sprite, 160, "Press any button");
    } else if (state_ == Ready) {
      card(sprite, TFT_NAVY);
      title(sprite, "Get Ready", 62, 2, TFT_NAVY);
      body(sprite, 100, "Watch the color");
      body(sprite, 116, "and tap fast");
      sprite.setTextColor(TFT_WHITE, TFT_NAVY);
      sprite.setCursor(28, 168);
      sprite.print("Round ");
      sprite.print(roundCount_ + 1);
    } else if (state_ == Waiting) {
      card(sprite, TFT_NAVY);
      title(sprite, "Wait", 72, 3, TFT_NAVY);
      body(sprite, 118, "Do not tap yet");
    } else if (state_ == Go) {
      card(sprite, TFT_BLACK);
      title(sprite, "GO!", 72, 4, TFT_BLACK);
      body(sprite, 126, "Tap BTN2");
    } else if (state_ == Result) {
      card(sprite, TFT_BLACK);
      title(sprite, "Result", 58, 3, TFT_BLACK);
      sprite.setTextColor(TFT_WHITE, TFT_BLACK);
      sprite.setTextSize(2);
      sprite.setCursor(24, 100);
      sprite.print(measuredReactionMs_);
      sprite.print(" ms");
      sprite.setTextSize(1);
      sprite.setCursor(22, 142);
      sprite.print("Best: ");
      sprite.print(bestReactionMs_);
      sprite.print(" ms");
      sprite.setCursor(22, 158);
      sprite.print("Rounds: ");
      sprite.print(roundCount_);
      body(sprite, 176, "BTN2 again");
      body(sprite, 190, "BTN1 title");
    } else if (state_ == FalseStart) {
      card(sprite, TFT_BLACK);
      title(sprite, "False", 68, 2, TFT_BLACK);
      title(sprite, "Start", 92, 2, TFT_BLACK);
      body(sprite, 144, "BTN2 retry");
      body(sprite, 158, "BTN1 title");
    }
  }

 private:
  enum State {
    Splash,
    Ready,
    Waiting,
    Go,
    Result,
    FalseStart
  };

  State state_ = Splash;
  uint32_t stateStartedMs_ = 0;
  uint32_t reactionStartMs_ = 0;
  uint32_t measuredReactionMs_ = 0;
  uint32_t bestReactionMs_ = 0;
  uint32_t roundCount_ = 0;
  uint32_t nextPromptMs_ = 0;

  void setState(State next, uint32_t nowMs) {
    state_ = next;
    stateStartedMs_ = nowMs;
  }

  void scheduleRound(uint32_t nowMs) {
    nextPromptMs_ = nowMs + random(900, 2800);
  }

  void card(TFT_eSprite &sprite, uint16_t color) const {
    sprite.fillRoundRect(12, 28, 111, 176, 12, color);
  }

  void title(TFT_eSprite &sprite, const char *text, int y, uint8_t size, uint16_t bg) const {
    sprite.setTextSize(size);
    sprite.setTextColor(TFT_WHITE, bg);
    int width = strlen(text) * 6 * size;
    sprite.setCursor((SCREEN_W - width) / 2, y);
    sprite.print(text);
  }

  void body(TFT_eSprite &sprite, int y, const char *text) const {
    sprite.setTextSize(1);
    sprite.setTextColor(TFT_WHITE, state_ == Result || state_ == FalseStart || state_ == Go ? TFT_BLACK : TFT_NAVY);
    int width = strlen(text) * 6;
    sprite.setCursor((SCREEN_W - width) / 2, y);
    sprite.print(text);
  }
};
