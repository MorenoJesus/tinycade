#pragma once

#include "../GameTypes.h"

class RunnerGame : public IGame {
 public:
  const char *name() const override { return "Runner"; }

  void init(GameContext &ctx) override {
    bestScore_ = 0;
    started_ = false;
    resetGame();
    runStartMs_ = ctx.nowMs;
  }

  void handleInput(GameContext &ctx, const InputState &input) override {
    (void)ctx;
    if (!started_) {
      if (input.flapPressed || input.actionPressed) {
        resetGame();
        started_ = true;
        runStartMs_ = ctx.nowMs;
        if (input.actionPressed) {
          jump();
        } else if (input.flapPressed) {
          ducking_ = true;
          boostActive_ = true;
          boostUntilMs_ = ctx.nowMs + 700;
        }
      }
      return;
    }

    if (gameOver_) {
      if (shortPressBoth(input)) {
        resetGame();
        runStartMs_ = ctx.nowMs;
        jump();
      }
      return;
    }

    if (input.actionPressed && onGround_) {
      jump();
    }

    if (input.flapDown && onGround_) {
      ducking_ = true;
      boostActive_ = true;
      boostUntilMs_ = ctx.nowMs + 700;
    } else if (!input.flapDown) {
      ducking_ = false;
    }
  }

  void update(GameContext &ctx) override {
    if (!started_ || gameOver_) {
      return;
    }

    if (boostActive_ && ctx.nowMs >= boostUntilMs_) {
      boostActive_ = false;
    }

    float speed = boostActive_ ? 3.3f : 2.2f;
    groundOffset_ = (groundOffset_ + (int)speed) % 16;

    if (!onGround_) {
      playerVel_ += 0.34f;
      playerY_ += playerVel_;
      if (playerY_ >= kGroundY - 20) {
        playerY_ = kGroundY - 20;
        playerVel_ = 0.0f;
        onGround_ = true;
        ducking_ = false;
      }
    }

    float maxX = obstacles_[0].x;
    for (int i = 0; i < 4; ++i) {
      if (obstacles_[i].x > maxX) {
        maxX = obstacles_[i].x;
      }
      obstacles_[i].x -= speed;
    }

    for (int i = 0; i < 4; ++i) {
      if (!obstacles_[i].scored && obstacles_[i].x + obstacles_[i].w < kPlayerX) {
        obstacles_[i].scored = true;
        ++score_;
        if (score_ > bestScore_) {
          bestScore_ = score_;
        }
      }

      if (obstacles_[i].x + obstacles_[i].w < -12) {
        resetObstacle(obstacles_[i], maxX + random(70, 112));
        maxX = obstacles_[i].x;
      }

      if (collidesWith(obstacles_[i])) {
        gameOver_ = true;
      }
    }
  }

  void draw(GameContext &ctx) override {
    TFT_eSprite &sprite = *ctx.sprite;
    if (!started_) {
      sprite.fillScreen(TFT_NAVY);
      sprite.setTextColor(TFT_WHITE, TFT_NAVY);
      sprite.setTextSize(2);
      sprite.setCursor(10, 50);
      sprite.print("Runner");
      sprite.setCursor(20, 74);
      sprite.print("Dash");
      sprite.setTextSize(1);
      sprite.setCursor(16, 120);
      sprite.print("BTN2 start / jump");
      sprite.setCursor(14, 134);
      sprite.print("BTN1 duck / boost");
      sprite.setCursor(16, 178);
      sprite.print("Press a button");
      return;
    }

    sprite.fillScreen(TFT_SKYBLUE);
    sprite.fillRect(0, kGroundY, SCREEN_W, SCREEN_H - kGroundY, TFT_DARKGREEN);
    sprite.fillRect(0, kGroundY - 6, SCREEN_W, 6, TFT_GREEN);
    for (int x = -groundOffset_; x < SCREEN_W; x += 16) {
      sprite.drawFastVLine(x, kGroundY - 4, 4, TFT_OLIVE);
    }

    for (int i = 0; i < 4; ++i) {
      drawObstacle(sprite, obstacles_[i]);
    }
    drawPlayer(sprite);

    sprite.fillRect(0, 0, SCREEN_W, 14, TFT_NAVY);
    sprite.setTextColor(TFT_WHITE, TFT_NAVY);
    sprite.setTextSize(1);
    sprite.setCursor(4, 3);
    sprite.print("Score:");
    sprite.print(score_);
    sprite.setCursor(76, 3);
    sprite.print("Best:");
    sprite.print(bestScore_);

    if (gameOver_) {
      sprite.fillRoundRect(12, 54, 111, 118, 10, TFT_MAROON);
      sprite.setTextColor(TFT_WHITE, TFT_MAROON);
      sprite.setTextSize(2);
      sprite.setCursor(24, 66);
      sprite.print("CRASH");
      sprite.setTextSize(1);
      sprite.setCursor(24, 96);
      sprite.print("Score: ");
      sprite.print(score_);
      sprite.setCursor(24, 112);
      sprite.print("Best: ");
      sprite.print(bestScore_);
      sprite.setCursor(18, 148);
      sprite.print("Tap both to retry");
    }
  }

 private:
  struct Obstacle {
    float x;
    int w;
    int h;
    bool high;
    bool scored;
  };

  static const int kGroundY = 202;
  static const uint32_t kStartGraceMs = 500;
  static constexpr float kPlayerX = 28.0f;
  Obstacle obstacles_[4];
  float playerY_ = kGroundY - 20;
  float playerVel_ = 0.0f;
  bool onGround_ = true;
  bool ducking_ = false;
  bool started_ = false;
  bool gameOver_ = false;
  bool boostActive_ = false;
  uint32_t boostUntilMs_ = 0;
  int score_ = 0;
  int bestScore_ = 0;
  int groundOffset_ = 0;
  uint32_t runStartMs_ = 0;

  void resetObstacle(Obstacle &obstacle, float startX) {
    obstacle.x = startX;
    obstacle.w = random(12, 18);
    obstacle.high = random(0, 3) == 0;
    obstacle.h = obstacle.high ? random(14, 24) : random(16, 28);
    obstacle.scored = false;
  }

  void resetGame() {
    playerY_ = kGroundY - 20;
    playerVel_ = 0.0f;
    onGround_ = true;
    ducking_ = false;
    gameOver_ = false;
    boostActive_ = false;
    score_ = 0;
    groundOffset_ = 0;
    resetObstacle(obstacles_[0], 160);
    resetObstacle(obstacles_[1], 238);
    resetObstacle(obstacles_[2], 322);
    resetObstacle(obstacles_[3], 418);
  }

  void jump() {
    playerVel_ = -5.9f;
    onGround_ = false;
    ducking_ = false;
  }

  bool collidesWith(const Obstacle &obstacle) const {
    if (runStartMs_ > 0 && millis() - runStartMs_ < kStartGraceMs) {
      return false;
    }

    int px = (int)kPlayerX;
    int py = (int)playerY_;
    int pw = ducking_ && onGround_ ? 20 : 16;
    int ph = ducking_ && onGround_ ? 12 : 20;
    if (ducking_ && onGround_) {
      py += 8;
    }

    int ox = (int)obstacle.x;
    bool overlapX = px + pw > ox && px < ox + obstacle.w;
    if (!overlapX) {
      return false;
    }

    if (obstacle.high) {
      return py < obstacle.h;
    }

    int obstacleTop = kGroundY - obstacle.h;
    return py + ph > obstacleTop;
  }

  void drawPlayer(TFT_eSprite &sprite) const {
    int px = (int)kPlayerX;
    int py = (int)playerY_;
    int pw = ducking_ && onGround_ ? 20 : 16;
    int ph = ducking_ && onGround_ ? 12 : 20;
    if (ducking_ && onGround_) {
      py += 8;
    }

    sprite.fillRoundRect(px, py, pw, ph, 4, gameOver_ ? TFT_RED : TFT_ORANGE);
    sprite.fillCircle(px + 4, py + 4, 2, TFT_BLACK);
    sprite.fillRect(px + 2, py + ph - 4, 5, 4, TFT_BROWN);
    sprite.fillRect(px + 9, py + ph - 4, 5, 4, TFT_BROWN);
  }

  void drawObstacle(TFT_eSprite &sprite, const Obstacle &obstacle) const {
    int x = (int)obstacle.x;
    if (obstacle.high) {
      sprite.fillRoundRect(x, 0, obstacle.w, obstacle.h, 2, TFT_PURPLE);
      sprite.fillRect(x - 1, obstacle.h - 3, obstacle.w + 2, 3, TFT_MAGENTA);
    } else {
      int y = kGroundY - obstacle.h;
      sprite.fillRoundRect(x, y, obstacle.w, obstacle.h, 2, TFT_GREEN);
      sprite.fillRect(x - 1, y, obstacle.w + 2, 4, TFT_DARKGREEN);
    }
  }
};
