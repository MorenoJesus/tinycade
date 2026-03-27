#pragma once

#include "../GameTypes.h"

class BreakoutGame : public IGame {
 public:
  const char *name() const override { return "Breakout"; }

  void init(GameContext &ctx) override {
    (void)ctx;
    resetGame();
  }

  void handleInput(GameContext &ctx, const InputState &input) override {
    (void)ctx;
    inputLeft_ = input.actionDown;
    inputRight_ = input.flapDown;

    if (gameOver_ && shortPressBoth(input)) {
      resetGame();
      launchBall();
      return;
    }

    if (readyToLaunch_ && (input.flapPressed || input.actionPressed)) {
      launchBall();
    }
  }

  void update(GameContext &ctx) override {
    (void)ctx;
    if (gameOver_) {
      return;
    }

    if (inputLeft_ && !inputRight_) {
      paddleVel_ = -kPaddleSpeed;
    } else if (inputRight_ && !inputLeft_) {
      paddleVel_ = kPaddleSpeed;
    } else {
      paddleVel_ *= 0.80f;
      if (fabs(paddleVel_) < 0.05f) {
        paddleVel_ = 0.0f;
      }
    }

    paddleX_ += paddleVel_;
    paddleX_ = constrain(paddleX_, 0.0f, (float)(SCREEN_W - kPaddleW));

    if (!ball_.launched) {
      stickBallToPaddle();
      return;
    }

    ball_.x += ball_.vx;
    ball_.y += ball_.vy;

    if (ball_.x - kBallRadius <= 0.0f) {
      ball_.x = kBallRadius;
      ball_.vx = fabs(ball_.vx);
    } else if (ball_.x + kBallRadius >= SCREEN_W - 1) {
      ball_.x = SCREEN_W - 1 - kBallRadius;
      ball_.vx = -fabs(ball_.vx);
    }

    if (ball_.y - kBallRadius <= 20.0f) {
      ball_.y = 20.0f + kBallRadius;
      ball_.vy = fabs(ball_.vy);
    }

    if (ballHitsPaddle()) {
      float hitPos = (ball_.x - paddleX_) / kPaddleW;
      float centered = (hitPos - 0.5f) * 2.0f;
      float speed = sqrt(ball_.vx * ball_.vx + ball_.vy * ball_.vy);
      float nextSpeed = min(speed + 0.08f, 5.2f);
      ball_.vx = centered * nextSpeed;
      ball_.vy = -sqrt(max(0.5f, nextSpeed * nextSpeed - ball_.vx * ball_.vx));
      ball_.y = kPaddleY - kBallRadius - 1.0f;
    }

    resolveBrickCollision();
    if (ball_.y - kBallRadius > SCREEN_H) {
      loseLife();
      return;
    }

    if (allBricksCleared()) {
      resetBricks();
      readyToLaunch_ = true;
      ball_.launched = false;
      stickBallToPaddle();
    }
  }

  void draw(GameContext &ctx) override {
    TFT_eSprite &sprite = *ctx.sprite;
    sprite.fillScreen(TFT_NAVY);
    sprite.fillRect(0, 0, SCREEN_W, 20, TFT_DARKCYAN);
    sprite.drawFastHLine(0, 20, SCREEN_W, TFT_WHITE);

    sprite.setTextColor(TFT_WHITE, TFT_DARKCYAN);
    sprite.setTextSize(1);
    sprite.setCursor(6, 6);
    sprite.print("BRK ");
    sprite.print(score_);
    sprite.setCursor(88, 6);
    sprite.print("L:");
    sprite.print(lives_);

    drawBricks(sprite);
    sprite.fillRoundRect((int)paddleX_, kPaddleY, kPaddleW, kPaddleH, 2, TFT_WHITE);
    sprite.drawRoundRect((int)paddleX_, kPaddleY, kPaddleW, kPaddleH, 2, TFT_DARKGREY);
    sprite.fillCircle((int)ball_.x, (int)ball_.y, (int)kBallRadius, TFT_WHITE);
    sprite.drawCircle((int)ball_.x, (int)ball_.y, (int)kBallRadius, TFT_LIGHTGREY);

    if (gameOver_) {
      sprite.fillRoundRect(14, 82, 107, 78, 8, TFT_RED);
      sprite.setTextColor(TFT_WHITE, TFT_RED);
      sprite.setTextSize(2);
      sprite.setCursor(31, 94);
      sprite.print("GAME");
      sprite.setCursor(31, 114);
      sprite.print("OVER");
      sprite.setTextSize(1);
      sprite.setCursor(18, 148);
      sprite.print("Tap both to retry");
      return;
    }

    if (readyToLaunch_) {
      sprite.fillRoundRect(12, 84, 111, 68, 8, TFT_DARKGREY);
      sprite.setTextColor(TFT_WHITE, TFT_DARKGREY);
      sprite.setTextSize(2);
      sprite.setCursor(17, 96);
      sprite.print("BREAKOUT");
      sprite.setTextSize(1);
      sprite.setCursor(22, 122);
      sprite.print("BTN2 left");
      sprite.setCursor(22, 134);
      sprite.print("BTN1 right");
      sprite.setCursor(20, 146);
      sprite.print("Tap to launch");
    }
  }

 private:
  struct Brick {
    int x;
    int y;
    int w;
    int h;
    bool alive;
  };

  struct Ball {
    float x;
    float y;
    float vx;
    float vy;
    bool launched;
  };

  static const int kBrickRows = 5;
  static const int kBrickCols = 5;
  static const int kBrickGap = 3;
  static const int kBrickTop = 28;
  static const int kBrickH = 12;
  static const int kPaddleW = 32;
  static const int kPaddleH = 6;
  static const int kPaddleY = SCREEN_H - 18;
  static constexpr float kPaddleSpeed = 3.4f;
  static constexpr float kBallRadius = 3.0f;

  Brick bricks_[kBrickRows * kBrickCols];
  Ball ball_;
  float paddleX_ = 0.0f;
  float paddleVel_ = 0.0f;
  int score_ = 0;
  int lives_ = 3;
  bool gameOver_ = false;
  bool readyToLaunch_ = true;
  bool inputLeft_ = false;
  bool inputRight_ = false;

  void resetBricks() {
    int brickW = (SCREEN_W - 12 - ((kBrickCols - 1) * kBrickGap)) / kBrickCols;
    int rowWidth = (brickW * kBrickCols) + ((kBrickCols - 1) * kBrickGap);
    int startX = (SCREEN_W - rowWidth) / 2;
    for (int row = 0; row < kBrickRows; ++row) {
      for (int col = 0; col < kBrickCols; ++col) {
        Brick &brick = bricks_[row * kBrickCols + col];
        brick.x = startX + col * (brickW + kBrickGap);
        brick.y = kBrickTop + row * (kBrickH + kBrickGap);
        brick.w = brickW;
        brick.h = kBrickH;
        brick.alive = true;
      }
    }
  }

  void resetBall(bool keepScore = false) {
    if (!keepScore) {
      lives_ = 3;
      score_ = 0;
    }
    ball_.launched = false;
    ball_.vx = 0.0f;
    ball_.vy = 0.0f;
    readyToLaunch_ = true;
    gameOver_ = false;
    stickBallToPaddle();
  }

  void resetGame() {
    paddleX_ = (SCREEN_W - kPaddleW) * 0.5f;
    paddleVel_ = 0.0f;
    resetBricks();
    resetBall(false);
  }

  void stickBallToPaddle() {
    ball_.x = paddleX_ + (kPaddleW * 0.5f);
    ball_.y = kPaddleY - kBallRadius - 1.0f;
  }

  void launchBall() {
    ball_.launched = true;
    readyToLaunch_ = false;
    ball_.vx = inputLeft_ && !inputRight_ ? -1.65f : 1.65f;
    if (inputLeft_ && inputRight_) {
      ball_.vx = 0.6f;
    }
    ball_.vy = -3.0f;
  }

  bool ballHitsPaddle() const {
    float ballLeft = ball_.x - kBallRadius;
    float ballRight = ball_.x + kBallRadius;
    float ballTop = ball_.y - kBallRadius;
    float ballBottom = ball_.y + kBallRadius;
    return ball_.vy > 0.0f &&
           ballBottom >= kPaddleY &&
           ballTop <= kPaddleY + kPaddleH &&
           ballRight >= paddleX_ &&
           ballLeft <= paddleX_ + kPaddleW;
  }

  void resolveBrickCollision() {
    float ballLeft = ball_.x - kBallRadius;
    float ballRight = ball_.x + kBallRadius;
    float ballTop = ball_.y - kBallRadius;
    float ballBottom = ball_.y + kBallRadius;

    for (int i = 0; i < kBrickRows * kBrickCols; ++i) {
      Brick &brick = bricks_[i];
      if (!brick.alive) {
        continue;
      }
      if (ballRight < brick.x || ballLeft > brick.x + brick.w || ballBottom < brick.y || ballTop > brick.y + brick.h) {
        continue;
      }

      brick.alive = false;
      ++score_;
      float dx = ball_.x - (brick.x + brick.w * 0.5f);
      float dy = ball_.y - (brick.y + brick.h * 0.5f);
      if (fabs(dx) > fabs(dy)) {
        ball_.vx = -ball_.vx;
      } else {
        ball_.vy = -ball_.vy;
      }
      return;
    }
  }

  bool allBricksCleared() const {
    for (int i = 0; i < kBrickRows * kBrickCols; ++i) {
      if (bricks_[i].alive) {
        return false;
      }
    }
    return true;
  }

  void loseLife() {
    --lives_;
    if (lives_ <= 0) {
      gameOver_ = true;
      ball_.launched = false;
      return;
    }
    resetBall(true);
  }

  void drawBricks(TFT_eSprite &sprite) const {
    const uint16_t palette[kBrickRows] = {TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN};
    for (int row = 0; row < kBrickRows; ++row) {
      for (int col = 0; col < kBrickCols; ++col) {
        const Brick &brick = bricks_[row * kBrickCols + col];
        if (!brick.alive) {
          continue;
        }
        uint16_t color = palette[row];
        sprite.fillRoundRect(brick.x, brick.y, brick.w, brick.h, 2, color);
        sprite.drawRect(brick.x, brick.y, brick.w, brick.h, TFT_BLACK);
      }
    }
  }
};
