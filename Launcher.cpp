#include "Launcher.h"

namespace {
const uint16_t kBg = TFT_NAVY;
const uint16_t kCard = TFT_DARKCYAN;
const uint16_t kAccent = TFT_YELLOW;
const uint16_t kText = TFT_WHITE;
}

Launcher::Launcher(IGame **games, int gameCount)
    : games_(games),
      gameCount_(gameCount),
      selectedGame_(0),
      activeGame_(-1),
      mode_(MODE_MENU),
      menuAnimOffset_(0) {}

void Launcher::begin() {
  selectedGame_ = 0;
  activeGame_ = -1;
  mode_ = MODE_MENU;
  menuAnimOffset_ = 0;
}

void Launcher::tick(GameContext &ctx, const InputState &input) {
  if (mode_ == MODE_MENU) {
    updateMenu(input);
    if (input.actionPressed && gameCount_ > 0) {
      startGame(ctx, selectedGame_);
      games_[activeGame_]->draw(ctx);
      drawExitOverlay(ctx, input);
      return;
    }
    drawMenu(ctx);
    return;
  }

  if (activeGame_ < 0 || activeGame_ >= gameCount_) {
    exitToMenu(ctx);
    drawMenu(ctx);
    return;
  }

  IGame *game = games_[activeGame_];
  ctx.requestExitToMenu = false;
  game->handleInput(ctx, input);
  game->update(ctx);

  if (input.bothHeldMs >= EXIT_HOLD_MS) {
    ctx.requestExitToMenu = true;
  }

  game->draw(ctx);
  drawExitOverlay(ctx, input);

  if (ctx.requestExitToMenu) {
    exitToMenu(ctx);
  }
}

void Launcher::startGame(GameContext &ctx, int index) {
  if (index < 0 || index >= gameCount_) {
    return;
  }

  activeGame_ = index;
  mode_ = MODE_GAME;
  ctx.requestExitToMenu = false;
  games_[activeGame_]->init(ctx);
}

void Launcher::exitToMenu(GameContext &ctx) {
  if (activeGame_ >= 0 && activeGame_ < gameCount_) {
    games_[activeGame_]->cleanup(ctx);
  }
  activeGame_ = -1;
  mode_ = MODE_MENU;
}

void Launcher::updateMenu(const InputState &input) {
  if (gameCount_ <= 0) {
    return;
  }

  if (input.flapPressed) {
    selectedGame_ = (selectedGame_ + 1) % gameCount_;
    const int visibleSlots = 4;
    if (selectedGame_ == 0) {
      menuAnimOffset_ = 0;
    } else if (selectedGame_ < (int)menuAnimOffset_) {
      menuAnimOffset_ = selectedGame_;
    } else if (selectedGame_ >= (int)menuAnimOffset_ + visibleSlots) {
      menuAnimOffset_ = selectedGame_ - visibleSlots + 1;
    }
  }
}

void Launcher::drawMenu(GameContext &ctx) const {
  TFT_eSprite &sprite = *ctx.sprite;
  sprite.fillScreen(kBg);

  for (int i = 0; i < 14; ++i) {
    int y = (i * 19 + (int)(ctx.nowMs / 30)) % SCREEN_H;
    int x = 8 + ((i * 31) % (SCREEN_W - 16));
    sprite.fillCircle(x, y, 1 + (i % 3 == 0), TFT_SKYBLUE);
  }

  sprite.fillRoundRect(8, 12, SCREEN_W - 16, 42, 10, kCard);
  sprite.setTextColor(kText, kCard);
  sprite.setTextSize(2);
  sprite.setCursor(18, 24);
  sprite.print("Tinycade");
  sprite.setTextSize(1);
  sprite.setCursor(20, 45);
  sprite.print("BTN1 next  BTN2 launch");

  const int cardTop = 70;
  const int cardHeight = 28;
  const int cardGap = 6;
  const int visibleSlots = 4;
  const int firstVisible = (int)menuAnimOffset_;
  const int lastVisible = min(gameCount_, firstVisible + visibleSlots);

  for (int i = firstVisible; i < lastVisible; ++i) {
    int visibleIndex = i - firstVisible;
    bool selected = i == selectedGame_;
    uint16_t fill = selected ? kAccent : TFT_BLACK;
    uint16_t text = selected ? TFT_BLACK : kText;
    int y = cardTop + visibleIndex * (cardHeight + cardGap);
    sprite.fillRoundRect(12, y, SCREEN_W - 24, cardHeight, 8, fill);
    sprite.setTextColor(text, fill);
    sprite.setTextSize(2);
    sprite.setCursor(20, y + 8);
    sprite.print(games_[i]->name());
  }

  if (gameCount_ > visibleSlots) {
    sprite.setTextColor(TFT_LIGHTGREY, kBg);
    sprite.setTextSize(1);
    if (firstVisible > 0) {
      sprite.setCursor(SCREEN_W - 16, cardTop - 10);
      sprite.print("^");
    }
    if (lastVisible < gameCount_) {
      sprite.setCursor(SCREEN_W - 16, cardTop + visibleSlots * (cardHeight + cardGap) - 4);
      sprite.print("v");
    }
  }

  sprite.setTextColor(TFT_LIGHTGREY, kBg);
  sprite.setTextSize(1);
  sprite.setCursor(12, SCREEN_H - 28);
  sprite.print("Hold both buttons in-game");
  sprite.setCursor(18, SCREEN_H - 16);
  sprite.print("for 1.5s to return here");
}

void Launcher::drawExitOverlay(GameContext &ctx, const InputState &input) const {
  if (!input.flapDown || !input.actionDown) {
    return;
  }

  TFT_eSprite &sprite = *ctx.sprite;
  const int barW = SCREEN_W - 20;
  const int fillW = (int)((min(input.bothHeldMs, EXIT_HOLD_MS) * barW) / EXIT_HOLD_MS);

  sprite.fillRoundRect(10, 6, barW, 14, 7, TFT_BLACK);
  sprite.fillRoundRect(12, 8, fillW - 4 > 0 ? fillW - 4 : 0, 10, 5, TFT_ORANGE);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  sprite.setTextSize(1);
  sprite.setCursor(28, 10);
  sprite.print("Hold to exit");
}
