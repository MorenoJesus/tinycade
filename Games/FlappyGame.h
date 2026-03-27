#pragma once

#include "../GameTypes.h"

class FlappyGame : public IGame {
 public:
  FlappyGame()
      : birdX_(40.0f),
        birdY_(SCREEN_H / 2.0f),
        birdVel_(0.0f),
        gameStarted_(false),
        gameOver_(false),
        scoreRecorded_(false),
        score_(0),
        highScore_(0) {
    for (int i = 0; i < kMaxTopScores; ++i) {
      topScores_[i] = 0;
    }
  }

  const char *name() const override { return "Flappy"; }

  void init(GameContext &ctx) override {
    (void)ctx;
    resetGame();
  }

  void handleInput(GameContext &ctx, const InputState &input) override {
    (void)ctx;

    if (!gameStarted_) {
      if (input.actionPressed) {
        gameStarted_ = true;
        birdVel_ = kFlapPower;
      }
      return;
    }

    if (gameOver_) {
      if (!scoreRecorded_) {
        updateTopScores(score_);
        scoreRecorded_ = true;
      }

      if (shortPressBoth(input)) {
        resetGame();
        gameStarted_ = true;
        birdVel_ = kFlapPower;
      }
      return;
    }

    if (input.actionPressed) {
      birdVel_ = kFlapPower;
    }
  }

  void update(GameContext &ctx) override {
    (void)ctx;
    if (!gameStarted_ || gameOver_) {
      return;
    }

    updateBird();
    updatePipe(pipeA_, pipeB_);
    updatePipe(pipeB_, pipeA_);

    if (checkCollision(pipeA_) || checkCollision(pipeB_)) {
      gameOver_ = true;
    }
  }

  void draw(GameContext &ctx) override {
    TFT_eSprite &sprite = *ctx.sprite;

    if (!gameStarted_) {
      drawStartScreen(sprite);
      return;
    }

    sprite.fillScreen(TFT_CYAN);
    drawPipe(sprite, pipeA_);
    drawPipe(sprite, pipeB_);
    drawBird(sprite);
    drawScore(sprite);

    if (gameOver_) {
      drawGameOverSummary(sprite);
    }
  }

  void cleanup(GameContext &ctx) override { (void)ctx; }

 private:
  struct PipeData {
    float x;
    int gapY;
    bool scored;
  };

  static const int kBirdRadius = 6;
  static const int kPipeWidth = 24;
  static const int kPipeGap = 70;
  static const int kPipeSpacing = 90;
  static constexpr float kGravity = 0.28f;
  static constexpr float kFlapPower = -4.8f;
  static constexpr float kPipeSpeed = 2.0f;
  static const int kMaxTopScores = 5;

  float birdX_;
  float birdY_;
  float birdVel_;
  PipeData pipeA_;
  PipeData pipeB_;
  bool gameStarted_;
  bool gameOver_;
  bool scoreRecorded_;
  int score_;
  int highScore_;
  int topScores_[kMaxTopScores];

  int randomGap() const { return random(50, SCREEN_H - 50); }

  void resetGame() {
    birdX_ = 40.0f;
    birdY_ = SCREEN_H / 2.0f;
    birdVel_ = 0.0f;

    pipeA_.x = SCREEN_W + 30;
    pipeA_.gapY = randomGap();
    pipeA_.scored = false;

    pipeB_.x = pipeA_.x + kPipeWidth + kPipeSpacing;
    pipeB_.gapY = randomGap();
    pipeB_.scored = false;

    score_ = 0;
    gameStarted_ = false;
    gameOver_ = false;
    scoreRecorded_ = false;
  }

  void updateTopScores(int newScore) {
    for (int i = 0; i < kMaxTopScores; ++i) {
      if (newScore > topScores_[i]) {
        for (int j = kMaxTopScores - 1; j > i; --j) {
          topScores_[j] = topScores_[j - 1];
        }
        topScores_[i] = newScore;
        break;
      }
    }
  }

  void drawBird(TFT_eSprite &sprite) const {
    int x = (int)birdX_;
    int y = (int)birdY_;
    sprite.fillCircle(x, y, kBirdRadius, TFT_YELLOW);
    sprite.fillCircle(x + 2, y - 2, 1, TFT_BLACK);
    sprite.fillTriangle(x + 5, y, x + 10, y - 2, x + 10, y + 2, TFT_ORANGE);
  }

  void drawPipe(TFT_eSprite &sprite, const PipeData &pipe) const {
    int x = (int)pipe.x;
    int gapTop = pipe.gapY - kPipeGap / 2;
    int gapBottom = pipe.gapY + kPipeGap / 2;

    sprite.fillRect(x, 0, kPipeWidth, gapTop, TFT_GREEN);
    sprite.fillRect(x - 2, gapTop - 6, kPipeWidth + 4, 6, TFT_DARKGREEN);

    sprite.fillRect(x, gapBottom, kPipeWidth, SCREEN_H - gapBottom, TFT_GREEN);
    sprite.fillRect(x - 2, gapBottom, kPipeWidth + 4, 6, TFT_DARKGREEN);
  }

  void drawScore(TFT_eSprite &sprite) const {
    sprite.setTextColor(TFT_WHITE, TFT_CYAN);
    sprite.setTextSize(2);
    sprite.setCursor(8, 8);
    sprite.print(score_);

    sprite.setTextSize(1);
    sprite.setCursor(84, 10);
    sprite.print("Hi:");
    sprite.print(highScore_);
  }

  void drawStartScreen(TFT_eSprite &sprite) const {
    sprite.fillScreen(TFT_CYAN);
    sprite.setTextColor(TFT_WHITE, TFT_CYAN);
    sprite.setTextSize(2);
    sprite.setCursor(18, 60);
    sprite.print("Flappy");
    sprite.setCursor(32, 84);
    sprite.print("Bird");

    sprite.setTextSize(1);
    sprite.setCursor(18, 126);
    sprite.print("BTN2 start / flap");
    sprite.setCursor(28, 140);
    sprite.print("BTN1 title screen");
    drawBird(sprite);
  }

  void drawGameOverSummary(TFT_eSprite &sprite) const {
    sprite.fillRoundRect(8, 55, 119, 130, 10, TFT_RED);
    sprite.setTextColor(TFT_WHITE, TFT_RED);
    sprite.setTextSize(2);
    sprite.setCursor(24, 65);
    sprite.print("OUCH");

    sprite.setTextSize(1);
    sprite.setCursor(22, 90);
    sprite.print("Score: ");
    sprite.print(score_);
    sprite.setCursor(22, 105);
    sprite.print("Session Hi: ");
    sprite.print(highScore_);
    sprite.setCursor(22, 125);
    sprite.print("Top 5");

    for (int i = 0; i < kMaxTopScores; ++i) {
      sprite.setCursor(22, 140 + (i * 10));
      sprite.print(i + 1);
      sprite.print(". ");
      sprite.print(topScores_[i]);
    }

    sprite.setCursor(18, 198);
    sprite.print("Tap both to retry");
  }

  void updateBird() {
    birdVel_ += kGravity;
    birdY_ += birdVel_;

    if (birdY_ < kBirdRadius) {
      birdY_ = kBirdRadius;
      birdVel_ = 0.0f;
    }

    if (birdY_ > SCREEN_H - kBirdRadius) {
      gameOver_ = true;
    }
  }

  void updatePipe(PipeData &pipe, PipeData &other) {
    pipe.x -= kPipeSpeed;
    if (pipe.x + kPipeWidth < 0) {
      pipe.x = other.x + kPipeWidth + kPipeSpacing;
      pipe.gapY = randomGap();
      pipe.scored = false;
    }

    if (!pipe.scored && birdX_ > pipe.x + kPipeWidth) {
      ++score_;
      pipe.scored = true;
      if (score_ > highScore_) {
        highScore_ = score_;
      }
    }
  }

  bool checkCollision(const PipeData &pipe) const {
    int birdLeft = (int)birdX_ - kBirdRadius;
    int birdRight = (int)birdX_ + kBirdRadius;
    int birdTop = (int)birdY_ - kBirdRadius;
    int birdBottom = (int)birdY_ + kBirdRadius;

    int pipeLeft = (int)pipe.x;
    int pipeRight = (int)pipe.x + kPipeWidth;
    int gapTop = pipe.gapY - kPipeGap / 2;
    int gapBottom = pipe.gapY + kPipeGap / 2;

    bool overlapX = birdRight > pipeLeft && birdLeft < pipeRight;
    bool hitTop = birdTop < gapTop;
    bool hitBottom = birdBottom > gapBottom;
    return overlapX && (hitTop || hitBottom);
  }
};
