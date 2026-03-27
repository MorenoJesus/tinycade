#pragma once

#include "../GameTypes.h"

class DinoGame : public IGame {
 public:
  const char *name() const override { return "Dino"; }

  void init(GameContext &ctx) override {
    highScore_ = 0;
    started_ = false;
    resetGame();
    runStartMs_ = ctx.nowMs;
  }

  void handleInput(GameContext &ctx, const InputState &input) override {
    if (!started_) {
      if (input.flapPressed || input.actionPressed) {
        resetGame();
        started_ = true;
        runStartMs_ = ctx.nowMs;
        if (input.actionPressed) {
          jump();
        }
      }
      return;
    }

    if (gameOver_) {
      if (shortPressBoth(input)) {
        resetGame();
        started_ = true;
        runStartMs_ = ctx.nowMs;
        jump();
      }
      return;
    }

    // Reversed controls:
    // BTN1 -> former button 2 action
    // BTN2 -> former button 1 action
    if (input.flapPressed) {
      ducking_ = true;
      boostActive_ = true;
      boostUntilMs_ = ctx.nowMs + 650;
    } else if (!input.flapDown) {
      ducking_ = false;
    }

    if (input.actionPressed && onGround_) {
      jump();
    }
  }

  void update(GameContext &ctx) override {
    if (!started_ || gameOver_) {
      return;
    }

    if (boostActive_ && ctx.nowMs >= boostUntilMs_) {
      boostActive_ = false;
    }

    const float speed = boostActive_ ? kBoostSpeed : kRunSpeed;
    groundOffset_ = (groundOffset_ + (int)speed) % 16;

    if (!onGround_) {
      playerVel_ += kGravity;
      playerY_ += playerVel_;
      if (playerY_ >= kGroundY - kPlayerH) {
        playerY_ = kGroundY - kPlayerH;
        playerVel_ = 0.0f;
        onGround_ = true;
      }
    }

    // Keep ducking active only while held on the ground.
    if (ducking_ && !onGround_) {
      ducking_ = false;
    }

    for (int i = 0; i < kObstacleCount; ++i) {
      obstacles_[i].x -= speed;
    }

    float maxX = obstacles_[0].x;
    for (int i = 1; i < kObstacleCount; ++i) {
      if (obstacles_[i].x > maxX) {
        maxX = obstacles_[i].x;
      }
    }

    for (int i = 0; i < kObstacleCount; ++i) {
      Obstacle &o = obstacles_[i];
      if (!o.scored && o.x + o.w < kPlayerX) {
        o.scored = true;
        ++score_;
        if (score_ > highScore_) {
          highScore_ = score_;
        }
      }

      if (o.x + o.w < -18) {
        spawnObstacle(o, maxX + random(64, 110));
        maxX = o.x;
      }

      if (collidesWith(o)) {
        gameOver_ = true;
      }
    }

  }

  void draw(GameContext &ctx) override {
    TFT_eSprite &sprite = *ctx.sprite;

    if (!started_) {
      drawIntro(sprite);
      return;
    }

    drawWorld(sprite);

    if (gameOver_) {
      drawGameOver(sprite);
    }
  }

 private:
  struct Obstacle {
    float x;
    int w;
    int h;
    int y;
    bool bird;
    bool scored;
  };

  static const int kGroundY = 202;
  static const int kPlayerX = 28;
  static const int kPlayerW = 16;
  static const int kPlayerH = 20;
  static const int kDuckH = 12;
  static const int kObstacleCount = 4;
  static const uint32_t kStartGraceMs = 500;
  static constexpr float kRunSpeed = 2.2f;
  static constexpr float kBoostSpeed = 3.3f;
  static constexpr float kGravity = 0.34f;
  static constexpr float kJumpVelocity = -5.9f;

  Obstacle obstacles_[kObstacleCount];
  float playerY_ = kGroundY - kPlayerH;
  float playerVel_ = 0.0f;
  bool onGround_ = true;
  bool ducking_ = false;
  bool started_ = false;
  bool gameOver_ = false;
  bool boostActive_ = false;
  uint32_t boostUntilMs_ = 0;
  int score_ = 0;
  int highScore_ = 0;
  int groundOffset_ = 0;
  uint32_t runStartMs_ = 0;

  void resetGame() {
    playerY_ = kGroundY - kPlayerH;
    playerVel_ = 0.0f;
    onGround_ = true;
    ducking_ = false;
    gameOver_ = false;
    boostActive_ = false;
    boostUntilMs_ = 0;
    score_ = 0;
    groundOffset_ = 0;

    spawnObstacle(obstacles_[0], 160);
    spawnObstacle(obstacles_[1], 232);
    spawnObstacle(obstacles_[2], 314);
    spawnObstacle(obstacles_[3], 406);
  }

  void jump() {
    playerVel_ = kJumpVelocity;
    onGround_ = false;
    ducking_ = false;
  }

  void spawnObstacle(Obstacle &o, float startX) {
    o.x = startX;
    o.scored = false;

    if (random(0, 5) == 0) {
      o.bird = true;
      o.w = random(16, 22);
      o.h = random(10, 15);
      o.y = random(120, 146);
    } else {
      o.bird = false;
      o.w = random(8, 16);
      o.h = random(18, 34);
      o.y = kGroundY - o.h;
    }
  }

  bool playerHit(int &px, int &py, int &pw, int &ph) const {
    px = kPlayerX;
    pw = ducking_ && onGround_ ? 20 : kPlayerW;
    ph = ducking_ && onGround_ ? kDuckH : kPlayerH;
    py = static_cast<int>(playerY_) + ((ducking_ && onGround_) ? 8 : 0);
    return true;
  }

  bool collidesWith(const Obstacle &o) const {
    if (runStartMs_ > 0 && millis() - runStartMs_ < kStartGraceMs) {
      return false;
    }

    int px = 0, py = 0, pw = 0, ph = 0;
    playerHit(px, py, pw, ph);

    const int ox = static_cast<int>(o.x);
    const bool overlapX = px + pw > ox && px < ox + o.w;
    if (!overlapX) {
      return false;
    }

    if (o.bird) {
      return py < o.y + o.h && py + ph > o.y;
    }

    return py + ph > o.y;
  }

  void drawPlayer(TFT_eSprite &sprite) const {
    int px = 0, py = 0, pw = 0, ph = 0;
    playerHit(px, py, pw, ph);

    sprite.fillRoundRect(px, py, pw, ph, 4, gameOver_ ? TFT_RED : TFT_ORANGE);
    sprite.fillCircle(px + 4, py + 4, 2, TFT_BLACK);
    sprite.fillRect(px + 2, py + ph - 4, 5, 4, TFT_BROWN);
    sprite.fillRect(px + 9, py + ph - 4, 5, 4, TFT_BROWN);

    if (ducking_ && onGround_) {
      sprite.fillRect(px + 10, py + 3, 7, 3, TFT_YELLOW);
    } else if (!onGround_) {
      sprite.fillTriangle(px + 14, py + 3, px + 20, py + 6, px + 14, py + 9, TFT_YELLOW);
    }
  }

  void drawObstacle(TFT_eSprite &sprite, const Obstacle &o) const {
    const int x = static_cast<int>(o.x);
    if (o.bird) {
      sprite.fillRoundRect(x, o.y, o.w, o.h, 3, TFT_DARKGREY);
      sprite.fillCircle(x + o.w - 4, o.y + 4, 2, TFT_WHITE);
      return;
    }

    sprite.fillRoundRect(x, o.y, o.w, o.h, 2, TFT_GREEN);
    sprite.fillRect(x - 1, o.y, o.w + 2, 4, TFT_DARKGREEN);
  }

  void drawWorld(TFT_eSprite &sprite) const {
    sprite.fillScreen(TFT_SKYBLUE);

    // Distant clouds and horizon.
    sprite.fillCircle(24, 36, 9, TFT_WHITE);
    sprite.fillCircle(34, 32, 11, TFT_WHITE);
    sprite.fillCircle(42, 38, 8, TFT_WHITE);
    sprite.fillCircle(92, 44, 7, TFT_WHITE);
    sprite.fillCircle(100, 40, 10, TFT_WHITE);

    sprite.fillRect(0, kGroundY, SCREEN_W, SCREEN_H - kGroundY, TFT_DARKGREEN);
    sprite.fillRect(0, kGroundY - 5, SCREEN_W, 5, TFT_GREEN);

    for (int x = -groundOffset_; x < SCREEN_W; x += 16) {
      sprite.drawFastVLine(x, kGroundY - 4, 4, TFT_OLIVE);
    }

    for (int i = 0; i < kObstacleCount; ++i) {
      drawObstacle(sprite, obstacles_[i]);
    }

    drawPlayer(sprite);

    sprite.fillRect(0, 0, SCREEN_W, 14, TFT_NAVY);
    sprite.setTextSize(1);
    sprite.setTextColor(TFT_WHITE, TFT_NAVY);
    sprite.setCursor(4, 3);
    sprite.print("Score:");
    sprite.print(score_);
    sprite.setCursor(76, 3);
    sprite.print("Hi:");
    sprite.print(highScore_);
  }

  void drawIntro(TFT_eSprite &sprite) const {
    sprite.fillScreen(TFT_SKYBLUE);
    sprite.fillCircle(24, 36, 9, TFT_WHITE);
    sprite.fillCircle(34, 32, 11, TFT_WHITE);
    sprite.fillCircle(42, 38, 8, TFT_WHITE);

    sprite.fillRect(0, kGroundY, SCREEN_W, SCREEN_H - kGroundY, TFT_DARKGREEN);
    sprite.fillRect(0, kGroundY - 5, SCREEN_W, 5, TFT_GREEN);

    sprite.setTextColor(TFT_BLACK, TFT_SKYBLUE);
    sprite.setTextSize(2);
    sprite.setCursor(18, 56);
    sprite.print("DINO");
    sprite.setCursor(18, 80);
    sprite.print("RUN");

    sprite.setTextSize(1);
    sprite.setTextColor(TFT_BLACK, TFT_SKYBLUE);
    sprite.setCursor(14, 120);
    sprite.print("BTN2 jump");
    sprite.setCursor(14, 134);
    sprite.print("BTN1 duck / boost");
    sprite.setCursor(14, 152);
    sprite.print("Press either to start");

    // Small idle dinosaur silhouette.
    sprite.fillRoundRect(92, 170, 18, 20, 4, TFT_ORANGE);
    sprite.fillCircle(96, 174, 2, TFT_BLACK);
    sprite.fillRect(100, 184, 4, 6, TFT_BROWN);
    sprite.fillRect(106, 184, 4, 6, TFT_BROWN);
  }

  void drawGameOver(TFT_eSprite &sprite) const {
    sprite.fillRoundRect(10, 56, 115, 124, 10, TFT_MAROON);
    sprite.setTextColor(TFT_WHITE, TFT_MAROON);
    sprite.setTextSize(2);
    sprite.setCursor(32, 70);
    sprite.print("GAME");
    sprite.setCursor(36, 92);
    sprite.print("OVER");

    sprite.setTextSize(1);
    sprite.setCursor(26, 126);
    sprite.print("Score: ");
    sprite.print(score_);
    sprite.setCursor(26, 140);
    sprite.print("Best: ");
    sprite.print(highScore_);
    sprite.setCursor(18, 170);
    sprite.print("Tap both to retry");
  }
};
