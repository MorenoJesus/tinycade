#pragma once

#include "../GameTypes.h"

class DashGame : public IGame {
 public:
  const char *name() const override { return "Dash"; }

  void init(GameContext &ctx) override {
    bestScore_ = 0;
    started_ = false;
    state_ = Intro;
    resetRun();
    runStartMs_ = ctx.nowMs;
  }

  void handleInput(GameContext &ctx, const InputState &input) override {
    if (!started_) {
      if (input.flapPressed || input.actionPressed) {
        resetRun();
        started_ = true;
        state_ = Running;
        runStartMs_ = ctx.nowMs;
        if (input.actionPressed) {
          jump();
        }
      }
      return;
    }

    if (gameOver_) {
      if (shortPressBoth(input)) {
        resetRun();
        started_ = true;
        state_ = Running;
        runStartMs_ = ctx.nowMs;
        jump();
      }
      return;
    }

    if (input.actionPressed && onGround_) {
      jump();
    }
  }

  void update(GameContext &ctx) override {
    (void)ctx;
    if (!started_ || gameOver_) {
      return;
    }

    const float speed = currentSpeed();
    scrollOffset_ = (scrollOffset_ + (int)speed) % 16;
    beatPulse_ = (beatPulse_ + 1) % 18;

    if (!onGround_) {
      playerVel_ += kGravity;
      playerY_ += playerVel_;
      if (playerY_ >= kGroundY - kPlayerSize) {
        playerY_ = kGroundY - kPlayerSize;
        playerVel_ = 0.0f;
        onGround_ = true;
      }
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
        if (score_ > bestScore_) {
          bestScore_ = score_;
        }
      }

      if (o.x + o.w < -24) {
        spawnObstacle(o, maxX + random(58, 96));
        maxX = o.x;
      }

      if (collidesWith(o)) {
        gameOver_ = true;
        state_ = GameOver;
      }
    }
  }

  void draw(GameContext &ctx) override {
    TFT_eSprite &sprite = *ctx.sprite;
    drawBackground(sprite);
    drawGround(sprite);
    drawDecor(sprite);

    for (int i = 0; i < kObstacleCount; ++i) {
      drawObstacle(sprite, obstacles_[i]);
    }

    drawPlayer(sprite);
    drawHud(sprite);

    if (state_ == Intro) {
      drawIntro(sprite);
    } else if (state_ == GameOver) {
      drawGameOver(sprite);
    }
  }

 private:
  enum State {
    Intro,
    Running,
    GameOver
  };

  enum ObstacleKind {
    Spike,
    Block
  };

  struct Obstacle {
    float x;
    int w;
    int h;
    bool scored;
    ObstacleKind kind;
  };

  static const int kPlayerX = 28;
  static const int kPlayerSize = 16;
  static const int kPlayerGlow = 22;
  static const int kGroundY = 206;
  static const int kObstacleCount = 5;
  static const uint32_t kStartGraceMs = 500;
  static constexpr float kGravity = 0.33f;
  static constexpr float kJumpVelocity = -5.9f;
  static constexpr float kBaseSpeed = 2.45f;
  static constexpr float kMaxSpeed = 4.1f;

  State state_ = Intro;
  Obstacle obstacles_[kObstacleCount];
  float playerY_ = kGroundY - kPlayerSize;
  float playerVel_ = 0.0f;
  bool onGround_ = true;
  bool started_ = false;
  bool gameOver_ = false;
  int score_ = 0;
  int bestScore_ = 0;
  int scrollOffset_ = 0;
  int beatPulse_ = 0;
  uint32_t runStartMs_ = 0;

  void resetRun() {
    playerY_ = kGroundY - kPlayerSize;
    playerVel_ = 0.0f;
    onGround_ = true;
    gameOver_ = false;
    score_ = 0;
    scrollOffset_ = 0;
    beatPulse_ = 0;

    spawnObstacle(obstacles_[0], 150);
    spawnObstacle(obstacles_[1], 224);
    spawnObstacle(obstacles_[2], 310);
    spawnObstacle(obstacles_[3], 394);
    spawnObstacle(obstacles_[4], 470);
  }

  void jump() {
    playerVel_ = kJumpVelocity;
    onGround_ = false;
  }

  float currentSpeed() const {
    float ramp = 0.06f * (float)score_;
    if (ramp > 1.45f) {
      ramp = 1.45f;
    }
    float speed = kBaseSpeed + ramp;
    if (speed > kMaxSpeed) {
      speed = kMaxSpeed;
    }
    return speed;
  }

  void spawnObstacle(Obstacle &o, float startX) {
    o.x = startX;
    o.scored = false;

    if (random(0, 6) == 0) {
      o.kind = Block;
      o.w = random(12, 18);
      o.h = random(20, 40);
    } else {
      o.kind = Spike;
      o.w = random(12, 16);
      o.h = random(14, 22);
    }
  }

  bool playerHit(int &px, int &py, int &pw, int &ph) const {
    px = kPlayerX;
    py = static_cast<int>(playerY_);
    pw = kPlayerSize;
    ph = kPlayerSize;
    return true;
  }

  bool collidesWith(const Obstacle &o) const {
    if (runStartMs_ > 0 && millis() - runStartMs_ < kStartGraceMs) {
      return false;
    }

    int px = 0;
    int py = 0;
    int pw = 0;
    int ph = 0;
    playerHit(px, py, pw, ph);

    const int ox = static_cast<int>(o.x);
    if (px + pw < ox || px > ox + o.w) {
      return false;
    }

    if (o.kind == Block) {
      const int top = kGroundY - o.h;
      return py + ph > top;
    }

    const int spikeCenter = ox + (o.w / 2);
    const int playerCenter = px + (pw / 2);
    const int halfWidth = max(1, o.w / 2);
    const int distance = abs(playerCenter - spikeCenter);
    if (distance > halfWidth) {
      return false;
    }

    const float heightScale = 1.0f - (float)distance / (float)halfWidth;
    const float spikeTop = (float)kGroundY - ((float)o.h * heightScale);
    return (float)(py + ph) > spikeTop;
  }

  void drawBackground(TFT_eSprite &sprite) const {
    sprite.fillScreen(TFT_BLACK);

    for (int y = 0; y < SCREEN_H; y += 16) {
      sprite.drawFastHLine(0, y, SCREEN_W, TFT_DARKGREY);
    }
    for (int x = scrollOffset_; x < SCREEN_W; x += 16) {
      sprite.drawFastVLine(x, 0, SCREEN_H, TFT_DARKGREY);
    }

    for (int i = 0; i < 14; ++i) {
      int sx = (i * 21 + beatPulse_ * 5) % SCREEN_W;
      int sy = (i * 37 + 11) % 170;
      sprite.fillCircle(sx, sy, 1, TFT_DARKCYAN);
    }

    sprite.fillRect(0, 0, SCREEN_W, 18, TFT_NAVY);
    sprite.fillRect(0, 18, SCREEN_W, 2, TFT_MAGENTA);
  }

  void drawGround(TFT_eSprite &sprite) const {
    sprite.fillRect(0, kGroundY, SCREEN_W, SCREEN_H - kGroundY, TFT_DARKGREY);
    sprite.fillRect(0, kGroundY - 3, SCREEN_W, 3, TFT_MAGENTA);

    for (int x = -scrollOffset_; x < SCREEN_W; x += 16) {
      sprite.drawFastVLine(x, kGroundY - 6, 6, TFT_PURPLE);
    }
  }

  void drawDecor(TFT_eSprite &sprite) const {
    sprite.fillCircle(20, 34, 6, TFT_DARKCYAN);
    sprite.fillCircle(28, 30, 9, TFT_CYAN);
    sprite.fillCircle(36, 34, 6, TFT_DARKCYAN);
    sprite.fillCircle(106, 52, 5, TFT_DARKCYAN);
    sprite.fillCircle(113, 49, 7, TFT_PINK);
  }

  void drawObstacle(TFT_eSprite &sprite, const Obstacle &o) const {
    const int x = static_cast<int>(o.x);
    if (o.kind == Block) {
      sprite.fillRoundRect(x, kGroundY - o.h, o.w, o.h, 2, TFT_CYAN);
      sprite.drawRoundRect(x, kGroundY - o.h, o.w, o.h, 2, TFT_WHITE);
      sprite.fillRect(x - 1, kGroundY - o.h, o.w + 2, 3, TFT_MAGENTA);
      return;
    }

    const int y = kGroundY - o.h;
    sprite.fillTriangle(x, kGroundY, x + (o.w / 2), y, x + o.w, kGroundY, TFT_RED);
    sprite.drawTriangle(x, kGroundY, x + (o.w / 2), y, x + o.w, kGroundY, TFT_ORANGE);
    sprite.fillRect(x - 1, kGroundY - 2, o.w + 2, 2, TFT_MAGENTA);
  }

  void drawPlayer(TFT_eSprite &sprite) const {
    int px = kPlayerX;
    int py = static_cast<int>(playerY_);

    sprite.fillRect(px - 2, py - 2, kPlayerGlow, kPlayerGlow, TFT_DARKCYAN);
    sprite.fillRoundRect(px, py, kPlayerSize, kPlayerSize, 3, TFT_YELLOW);
    sprite.fillRoundRect(px + 3, py + 3, 4, 4, 1, TFT_BLACK);
    sprite.fillRect(px + 8, py + 8, 4, 4, TFT_BLUE);

    if (!onGround_) {
      sprite.fillRect(px + 1, py + 15, 14, 2, TFT_WHITE);
    } else {
      sprite.fillRect(px + 1, py + 15, 14, 2, TFT_MAGENTA);
    }
  }

  void drawHud(TFT_eSprite &sprite) const {
    sprite.setTextSize(1);
    sprite.setTextColor(TFT_WHITE, TFT_NAVY);
    sprite.setCursor(6, 4);
    sprite.print("DASH ");
    sprite.print(score_);
    sprite.setCursor(74, 4);
    sprite.print("Hi:");
    sprite.print(bestScore_);
  }

  void drawIntro(TFT_eSprite &sprite) const {
    sprite.fillRoundRect(12, 36, 111, 150, 12, TFT_NAVY);
    sprite.fillRoundRect(18, 42, 99, 138, 10, TFT_BLACK);
    sprite.setTextColor(TFT_WHITE, TFT_BLACK);
    sprite.setTextSize(2);
    sprite.setCursor(27, 58);
    sprite.print("GEOM");
    sprite.setCursor(24, 82);
    sprite.print("DASH");
    sprite.setTextSize(1);
    sprite.setCursor(18, 114);
    sprite.print("BTN2 jump / start");
    sprite.setCursor(18, 128);
    sprite.print("BTN1 return / title");
    sprite.setCursor(18, 146);
    sprite.print("Tap the cube");
    sprite.setCursor(18, 158);
    sprite.print("and time the spikes");

    sprite.fillRoundRect(88, 126, 14, 14, 2, TFT_YELLOW);
    sprite.fillRect(92, 130, 4, 4, TFT_BLACK);
    sprite.fillTriangle(100, 132, 110, 128, 110, 136, TFT_RED);
    sprite.fillTriangle(110, 132, 118, 124, 118, 140, TFT_MAGENTA);
  }

  void drawGameOver(TFT_eSprite &sprite) const {
    sprite.fillRoundRect(10, 44, 115, 132, 10, TFT_MAROON);
    sprite.setTextColor(TFT_WHITE, TFT_MAROON);
    sprite.setTextSize(2);
    sprite.setCursor(24, 58);
    sprite.print("CRASH");
    sprite.setCursor(22, 82);
    sprite.print("DASH");
    sprite.setTextSize(1);
    sprite.setCursor(22, 112);
    sprite.print("Score: ");
    sprite.print(score_);
    sprite.setCursor(22, 126);
    sprite.print("Best: ");
    sprite.print(bestScore_);
    sprite.setCursor(18, 156);
    sprite.print("Tap both to retry");
  }
};
