#pragma once

#include "../GameTypes.h"

class SnakeGame : public IGame {
 public:
  const char *name() const override { return "Snake"; }

  void init(GameContext &ctx) override {
    (void)ctx;
    highScore_ = 0;
    shouldBlink_ = false;
    state_ = Intro;
    resetRun();
  }

  void handleInput(GameContext &ctx, const InputState &input) override {
    (void)ctx;
    if (state_ == Intro) {
      if (input.flapPressed || input.actionPressed) {
        startRun(ctx.nowMs);
      }
      return;
    }

    if (state_ == GameOver) {
      if (shortPressBoth(input)) {
        startRun(ctx.nowMs);
      }
      return;
    }

    if (state_ != Running) {
      return;
    }

    if (input.actionPressed) {
      direction_ = turnLeft(direction_);
    }

    if (input.flapPressed) {
      direction_ = turnRight(direction_);
    }
  }

  void update(GameContext &ctx) override {
    shouldBlink_ = ((ctx.nowMs / 400) % 2) == 0;
    if (state_ != Running) {
      return;
    }

    if (ctx.nowMs - lastStepMs_ < stepIntervalMs_) {
      return;
    }

    lastStepMs_ = ctx.nowMs;
    if (!advanceSnake()) {
      state_ = GameOver;
    }
  }

  void draw(GameContext &ctx) override {
    TFT_eSprite &sprite = *ctx.sprite;
    drawBoard(sprite);

    if (state_ == Intro) {
      sprite.setTextColor(TFT_YELLOW, TFT_NAVY);
      sprite.setTextSize(2);
      sprite.setCursor(28, 78);
      sprite.print("Snake");
      sprite.setTextSize(1);
      sprite.setTextColor(TFT_WHITE, TFT_NAVY);
      sprite.setCursor(22, 116);
      sprite.print("BTN2 left");
      sprite.setCursor(22, 130);
      sprite.print("BTN1 right");
      sprite.setCursor(18, 154);
      sprite.print(shouldBlink_ ? "Press either to play" : "                    ");
    } else if (state_ == GameOver) {
      sprite.fillRoundRect(14, 80, 107, 82, 8, TFT_MAROON);
      sprite.setTextColor(TFT_WHITE, TFT_MAROON);
      sprite.setTextSize(2);
      sprite.setCursor(34, 92);
      sprite.print("GAME");
      sprite.setCursor(30, 112);
      sprite.print("OVER");
      sprite.setTextSize(1);
      sprite.setCursor(32, 138);
      sprite.print("Score: ");
      sprite.print(score_);
      sprite.setCursor(18, 176);
      sprite.print("Tap both to retry");
    }
  }

 private:
  enum State {
    Intro,
    Running,
    GameOver
  };

  enum Direction {
    Up,
    Right,
    Down,
    Left
  };

  struct Cell {
    uint8_t x;
    uint8_t y;
  };

  static const int kHudH = 20;
  static const int kCellSize = 10;
  static const int kGridCols = 13;
  static const int kGridRows = 22;
  static const int kBoardW = kGridCols * kCellSize;
  static const int kBoardH = kGridRows * kCellSize;
  static const int kBoardX = (SCREEN_W - kBoardW) / 2;
  static const int kBoardY = kHudH;
  static const int kMaxSnake = kGridCols * kGridRows;
  static const uint32_t kInitialStepMs = 180;
  static const uint32_t kMinStepMs = 75;

  State state_ = Intro;
  Direction direction_ = Right;
  Cell snake_[kMaxSnake];
  Cell food_ = {0, 0};
  uint16_t snakeLength_ = 3;
  uint16_t score_ = 0;
  uint16_t highScore_ = 0;
  uint32_t lastStepMs_ = 0;
  uint32_t stepIntervalMs_ = kInitialStepMs;
  bool shouldBlink_ = false;

  bool isOnSnake(uint8_t x, uint8_t y) const {
    for (uint16_t i = 0; i < snakeLength_; ++i) {
      if (snake_[i].x == x && snake_[i].y == y) {
        return true;
      }
    }
    return false;
  }

  void spawnFood() {
    do {
      food_.x = (uint8_t)random(kGridCols);
      food_.y = (uint8_t)random(kGridRows);
    } while (isOnSnake(food_.x, food_.y));
  }

  void resetRun() {
    uint8_t startX = kGridCols / 2;
    uint8_t startY = kGridRows / 2;
    snakeLength_ = 3;
    score_ = 0;
    stepIntervalMs_ = kInitialStepMs;
    direction_ = Right;

    for (uint16_t i = 0; i < snakeLength_; ++i) {
      snake_[i] = {(uint8_t)(startX - i), startY};
    }
    spawnFood();
  }

  void startRun(uint32_t nowMs) {
    resetRun();
    lastStepMs_ = nowMs;
    state_ = Running;
  }

  Direction turnLeft(Direction dir) const {
    if (dir == Up) return Left;
    if (dir == Left) return Down;
    if (dir == Down) return Right;
    return Up;
  }

  Direction turnRight(Direction dir) const {
    if (dir == Up) return Right;
    if (dir == Right) return Down;
    if (dir == Down) return Left;
    return Up;
  }

  bool advanceSnake() {
    Cell head = snake_[0];
    int nextX = head.x;
    int nextY = head.y;

    if (direction_ == Up) --nextY;
    if (direction_ == Right) ++nextX;
    if (direction_ == Down) ++nextY;
    if (direction_ == Left) --nextX;

    if (nextX < 0 || nextX >= kGridCols || nextY < 0 || nextY >= kGridRows) {
      return false;
    }

    bool willGrow = nextX == food_.x && nextY == food_.y;
    uint16_t bodyLimit = willGrow ? snakeLength_ : snakeLength_ - 1;
    for (uint16_t i = 0; i < bodyLimit; ++i) {
      if (snake_[i].x == nextX && snake_[i].y == nextY) {
        return false;
      }
    }

    if (willGrow && snakeLength_ < kMaxSnake) {
      ++snakeLength_;
    }

    for (int i = snakeLength_ - 1; i > 0; --i) {
      snake_[i] = snake_[i - 1];
    }
    snake_[0] = {(uint8_t)nextX, (uint8_t)nextY};

    if (willGrow) {
      ++score_;
      if (score_ > highScore_) {
        highScore_ = score_;
      }
      int slowed = (int)kInitialStepMs - ((int)score_ * 4);
      stepIntervalMs_ = (uint32_t)max((int)kMinStepMs, slowed);
      spawnFood();
    }
    return true;
  }

  void drawCell(TFT_eSprite &sprite, uint8_t x, uint8_t y, uint16_t color) const {
    sprite.fillRect(kBoardX + x * kCellSize + 1, kBoardY + y * kCellSize + 1, kCellSize - 2, kCellSize - 2, color);
  }

  void drawBoard(TFT_eSprite &sprite) const {
    sprite.fillScreen(TFT_NAVY);
    sprite.fillRect(0, 0, SCREEN_W, kHudH, TFT_DARKGREY);
    sprite.setTextColor(TFT_WHITE, TFT_DARKGREY);
    sprite.setTextSize(1);
    sprite.setCursor(4, 5);
    sprite.print("Snake ");
    sprite.print(score_);
    sprite.print(" Hi:");
    sprite.print(highScore_);

    sprite.fillRect(kBoardX, kBoardY, kBoardW, kBoardH, TFT_BLACK);
    sprite.drawRect(kBoardX, kBoardY, kBoardW, kBoardH, TFT_DARKGREY);

    for (uint8_t x = 0; x < kGridCols; ++x) {
      for (uint8_t y = 0; y < kGridRows; ++y) {
        if (((x + y) % 2) == 0) {
          sprite.fillRect(kBoardX + x * kCellSize, kBoardY + y * kCellSize, kCellSize, kCellSize, TFT_DARKGREEN);
        }
      }
    }

    drawCell(sprite, food_.x, food_.y, TFT_RED);
    for (int i = snakeLength_ - 1; i >= 0; --i) {
      drawCell(sprite, snake_[i].x, snake_[i].y, i == 0 ? TFT_GREENYELLOW : TFT_GREEN);
    }
  }
};
