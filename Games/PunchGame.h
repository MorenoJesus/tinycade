#pragma once

#include "../GameTypes.h"

class PunchGame : public IGame {
 public:
  const char *name() const override { return "Punch"; }

  void init(GameContext &ctx) override {
    highScore_ = 0;
    state_ = Intro;
    resetRound();
    runStartMs_ = ctx.nowMs;
  }

  void handleInput(GameContext &ctx, const InputState &input) override {
    if (state_ == Intro) {
      if (input.flapPressed || input.actionPressed) {
        resetRound();
        state_ = Running;
        runStartMs_ = ctx.nowMs;
      }
      return;
    }

    if (state_ == GameOver) {
      if (shortPressBoth(input)) {
        resetRound();
        state_ = Running;
        runStartMs_ = ctx.nowMs;
      }
      return;
    }

    if (state_ != Running) {
      return;
    }

    // Reversed in-game mapping:
    // BTN2 attacks left, BTN1 attacks right.
    if (input.actionPressed) {
      attack(ctx, LeftLane);
    }

    if (input.flapPressed) {
      attack(ctx, RightLane);
    }
  }

  void update(GameContext &ctx) override {
    if (state_ != Running) {
      pulseTick_ = (pulseTick_ + 1) % 40;
      updateBursts(ctx.nowMs);
      return;
    }

    pulseTick_ = (pulseTick_ + 1) % 40;
    if (playerFlashMs_ > 0 && ctx.nowMs >= playerFlashMs_) {
      playerFlashMs_ = 0;
    }
    updateBursts(ctx.nowMs);

    if (ctx.nowMs - lastSpawnMs_ >= spawnIntervalMs_ &&
        runStartMs_ > 0 && ctx.nowMs - runStartMs_ >= kStartGraceMs) {
      spawnEnemy(ctx.nowMs);
      lastSpawnMs_ = ctx.nowMs;
    }

    bool anyAlive = false;
    for (int i = 0; i < kEnemyCount; ++i) {
      Enemy &enemy = enemies_[i];
      if (!enemy.alive) {
        continue;
      }

      anyAlive = true;
      enemy.progress += enemy.speed;
      if (enemy.progress >= kReachProgress) {
        enemy.alive = false;
        combo_ = 0;
        if (lives_ > 0) {
          --lives_;
        }
        playerFlashMs_ = ctx.nowMs + 120;
        if (lives_ <= 0) {
          state_ = GameOver;
        }
      }
    }

    if (state_ == Running && !anyAlive &&
        ctx.nowMs - lastSpawnMs_ > spawnIntervalMs_ &&
        runStartMs_ > 0 && ctx.nowMs - runStartMs_ >= kStartGraceMs) {
      spawnEnemy(ctx.nowMs);
      lastSpawnMs_ = ctx.nowMs;
    }

    updateDifficulty();
  }

  void draw(GameContext &ctx) override {
    TFT_eSprite &sprite = *ctx.sprite;
    drawBackground(sprite);
    drawLanes(sprite);
    drawPlayer(sprite);
    drawEnemies(sprite);
    drawBursts(sprite);
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

  enum Lane {
    LeftLane,
    RightLane
  };

  struct Enemy {
    bool alive;
    Lane lane;
    float progress;
    float speed;
    bool heavy;
  };

  struct Burst {
    bool active;
    int x;
    int y;
    uint16_t color;
    uint32_t untilMs;
    uint8_t size;
  };

  static const int kEnemyCount = 6;
  static const int kBurstCount = 6;
  static const int kPlayerX = 67;
  static const int kPlayerY = 150;
  static const int kReachProgress = 88;
  static const uint32_t kStartGraceMs = 500;

  State state_ = Intro;
  Enemy enemies_[kEnemyCount];
  Burst bursts_[kBurstCount];
  int score_ = 0;
  int highScore_ = 0;
  int combo_ = 0;
  int lives_ = 3;
  uint32_t lastSpawnMs_ = 0;
  uint32_t spawnIntervalMs_ = 900;
  uint32_t playerFlashMs_ = 0;
  uint32_t runStartMs_ = 0;
  int pulseTick_ = 0;

  void resetRound() {
    for (int i = 0; i < kEnemyCount; ++i) {
      enemies_[i].alive = false;
      enemies_[i].lane = LeftLane;
      enemies_[i].progress = 0.0f;
      enemies_[i].speed = 1.0f;
      enemies_[i].heavy = false;
    }
    for (int i = 0; i < kBurstCount; ++i) {
      bursts_[i].active = false;
      bursts_[i].x = 0;
      bursts_[i].y = 0;
      bursts_[i].color = TFT_WHITE;
      bursts_[i].untilMs = 0;
      bursts_[i].size = 0;
    }
    score_ = 0;
    combo_ = 0;
    lives_ = 3;
    spawnIntervalMs_ = 900;
    lastSpawnMs_ = 0;
    playerFlashMs_ = 0;
  }

  void updateDifficulty() {
    uint32_t target = 900;
    if (score_ > 10) {
      target = 780;
    }
    if (score_ > 20) {
      target = 690;
    }
    if (score_ > 35) {
      target = 610;
    }
    if (score_ > 50) {
      target = 540;
    }
    spawnIntervalMs_ = target;
  }

  void spawnEnemy(uint32_t nowMs) {
    for (int i = 0; i < kEnemyCount; ++i) {
      if (enemies_[i].alive) {
        continue;
      }
      enemies_[i].alive = true;
      enemies_[i].lane = random(0, 2) == 0 ? LeftLane : RightLane;
      enemies_[i].progress = 0.0f;
      enemies_[i].heavy = random(0, 5) == 0 && score_ > 12;
      enemies_[i].speed = enemies_[i].heavy ? 1.45f : 1.9f + min(score_ * 0.02f, 0.8f);
      playerFlashMs_ = nowMs > playerFlashMs_ ? playerFlashMs_ : nowMs;
      return;
    }
  }

  void attack(GameContext &ctx, Lane lane) {
    int bestIndex = -1;
    float bestProgress = -1.0f;

    for (int i = 0; i < kEnemyCount; ++i) {
      Enemy &enemy = enemies_[i];
      if (!enemy.alive || enemy.lane != lane) {
        continue;
      }
      if (enemy.progress > bestProgress) {
        bestProgress = enemy.progress;
        bestIndex = i;
      }
    }

    if (bestIndex >= 0 && bestProgress >= 34.0f) {
      Enemy &enemy = enemies_[bestIndex];
      int x = enemyScreenX(enemy);
      int y = enemyScreenY(enemy) - 6;
      uint16_t color = enemy.heavy ? TFT_ORANGE : TFT_GREENYELLOW;
      enemy.alive = false;
      combo_ += 1;
      score_ += enemy.heavy ? 3 : 1;
      if (combo_ > 0 && combo_ % 5 == 0) {
        score_ += 2;
      }
      if (score_ > highScore_) {
        highScore_ = score_;
      }
      spawnBurst(x, y, color, ctx.nowMs, enemy.heavy ? 12 : 9);
      playerFlashMs_ = ctx.nowMs + 90;
      return;
    }

    combo_ = 0;
    if (lives_ > 0) {
      --lives_;
    }
    playerFlashMs_ = ctx.nowMs + 150;
    if (lives_ <= 0) {
      state_ = GameOver;
    }
  }

  void drawBackground(TFT_eSprite &sprite) const {
    sprite.fillScreen(TFT_BLACK);

    for (int y = 0; y < SCREEN_H; y += 20) {
      sprite.drawFastHLine(0, y, SCREEN_W, TFT_DARKGREY);
    }

    for (int x = 0; x < SCREEN_W; x += 18) {
      sprite.drawFastVLine(x, 0, SCREEN_H, TFT_DARKGREY);
    }

    uint16_t glow = pulseTick_ < 20 ? TFT_DARKCYAN : TFT_DARKGREEN;
    sprite.fillRect(0, 0, SCREEN_W, 18, TFT_NAVY);
    sprite.fillRect(0, 18, SCREEN_W, 2, glow);
  }

  void drawLanes(TFT_eSprite &sprite) const {
    sprite.drawFastVLine(32, 42, 160, TFT_DARKCYAN);
    sprite.drawFastVLine(102, 42, 160, TFT_DARKCYAN);
    sprite.drawFastHLine(16, 168, 103, TFT_DARKGREY);
  }

  void drawPlayer(TFT_eSprite &sprite) const {
    uint16_t color = playerFlashMs_ > 0 ? TFT_RED : TFT_WHITE;
    sprite.fillCircle(kPlayerX, kPlayerY - 22, 8, color);
    sprite.drawLine(kPlayerX, kPlayerY - 14, kPlayerX, kPlayerY + 10, color);
    sprite.drawLine(kPlayerX, kPlayerY - 8, kPlayerX - 14, kPlayerY - 2, color);
    sprite.drawLine(kPlayerX, kPlayerY - 8, kPlayerX + 14, kPlayerY - 2, color);
    sprite.drawLine(kPlayerX, kPlayerY + 10, kPlayerX - 10, kPlayerY + 24, color);
    sprite.drawLine(kPlayerX, kPlayerY + 10, kPlayerX + 10, kPlayerY + 24, color);
  }

  void drawEnemies(TFT_eSprite &sprite) const {
    for (int i = 0; i < kEnemyCount; ++i) {
      const Enemy &enemy = enemies_[i];
      if (!enemy.alive) {
        continue;
      }

      int x = enemyScreenX(enemy);
      int y = enemyScreenY(enemy);
      uint16_t color = enemy.heavy ? TFT_ORANGE : TFT_GREENYELLOW;

      sprite.fillCircle(x, y - 20, enemy.heavy ? 7 : 6, color);
      sprite.drawLine(x, y - 14, x, y + 8, color);
      sprite.drawLine(x, y - 8, x - 10, y - 1, color);
      sprite.drawLine(x, y - 8, x + 10, y - 1, color);
      sprite.drawLine(x, y + 8, x - 8, y + 20, color);
      sprite.drawLine(x, y + 8, x + 8, y + 20, color);
    }
  }

  void drawBursts(TFT_eSprite &sprite) const {
    for (int i = 0; i < kBurstCount; ++i) {
      const Burst &burst = bursts_[i];
      if (!burst.active) {
        continue;
      }

      int s = burst.size;
      sprite.fillCircle(burst.x, burst.y, max(1, s / 3), burst.color);
      sprite.drawLine(burst.x - s, burst.y, burst.x + s, burst.y, burst.color);
      sprite.drawLine(burst.x, burst.y - s, burst.x, burst.y + s, burst.color);
      sprite.drawLine(burst.x - s + 2, burst.y - s + 2, burst.x + s - 2, burst.y + s - 2, burst.color);
      sprite.drawLine(burst.x - s + 2, burst.y + s - 2, burst.x + s - 2, burst.y - s + 2, burst.color);
    }
  }

  void drawHud(TFT_eSprite &sprite) const {
    sprite.setTextSize(1);
    sprite.setTextColor(TFT_WHITE, TFT_NAVY);
    sprite.setCursor(4, 4);
    sprite.print("Punch ");
    sprite.print(score_);
    sprite.setCursor(56, 4);
    sprite.print("Hi:");
    sprite.print(highScore_);
    sprite.setCursor(96, 4);
    sprite.print("L:");
    sprite.print(lives_);

    sprite.setTextColor(TFT_CYAN, TFT_BLACK);
    sprite.setCursor(8, SCREEN_H - 12);
    sprite.print("Combo ");
    sprite.print(combo_);
  }

  void drawIntro(TFT_eSprite &sprite) const {
    sprite.fillRoundRect(10, 42, 115, 126, 10, TFT_NAVY);
    sprite.setTextColor(TFT_WHITE, TFT_NAVY);
    sprite.setTextSize(2);
    sprite.setCursor(24, 56);
    sprite.print("PUNCH");
    sprite.setCursor(28, 80);
    sprite.print("DUEL");
    sprite.setTextSize(1);
    sprite.setCursor(18, 112);
    sprite.print("BTN2 hit left");
    sprite.setCursor(18, 126);
    sprite.print("BTN1 hit right");
    sprite.setCursor(18, 142);
    sprite.print("Strike late, not early");
    sprite.setCursor(18, 156);
    sprite.print("Press any button");
  }

  void drawGameOver(TFT_eSprite &sprite) const {
    sprite.fillRoundRect(10, 42, 115, 130, 10, TFT_MAROON);
    sprite.setTextColor(TFT_WHITE, TFT_MAROON);
    sprite.setTextSize(2);
    sprite.setCursor(22, 58);
    sprite.print("KNOCK");
    sprite.setCursor(26, 82);
    sprite.print("OUT");
    sprite.setTextSize(1);
    sprite.setCursor(22, 114);
    sprite.print("Score: ");
    sprite.print(score_);
    sprite.setCursor(22, 128);
    sprite.print("Best: ");
    sprite.print(highScore_);
    sprite.setCursor(22, 144);
    sprite.print("Combo: ");
    sprite.print(combo_);
    sprite.setCursor(18, 168);
    sprite.print("Tap both to retry");
  }

  int enemyScreenX(const Enemy &enemy) const {
    int startX = enemy.lane == LeftLane ? 10 : SCREEN_W - 10;
    int targetX = enemy.lane == LeftLane ? 48 : 86;
    return startX + (int)((targetX - startX) * (enemy.progress / kReachProgress));
  }

  int enemyScreenY(const Enemy &enemy) const {
    return 148 + (enemy.heavy ? -6 : 0);
  }

  void spawnBurst(int x, int y, uint16_t color, uint32_t nowMs, uint8_t size) {
    for (int i = 0; i < kBurstCount; ++i) {
      if (bursts_[i].active) {
        continue;
      }
      bursts_[i].active = true;
      bursts_[i].x = x;
      bursts_[i].y = y;
      bursts_[i].color = color;
      bursts_[i].untilMs = nowMs + 110;
      bursts_[i].size = size;
      return;
    }

    bursts_[0].active = true;
    bursts_[0].x = x;
    bursts_[0].y = y;
    bursts_[0].color = color;
    bursts_[0].untilMs = nowMs + 110;
    bursts_[0].size = size;
  }

  void updateBursts(uint32_t nowMs) {
    for (int i = 0; i < kBurstCount; ++i) {
      Burst &burst = bursts_[i];
      if (!burst.active) {
        continue;
      }
      if (nowMs >= burst.untilMs) {
        burst.active = false;
      }
    }
  }
};
