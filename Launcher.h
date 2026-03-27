#pragma once

#include "GameTypes.h"

class Launcher {
 public:
  Launcher(IGame **games, int gameCount);

  void begin();
  void tick(GameContext &ctx, const InputState &input);

 private:
  IGame **games_;
  int gameCount_;
  int selectedGame_;
  int activeGame_;
  AppMode mode_;
  uint32_t menuAnimOffset_;

  void startGame(GameContext &ctx, int index);
  void exitToMenu(GameContext &ctx);
  void updateMenu(const InputState &input);
  void drawMenu(GameContext &ctx) const;
  void drawExitOverlay(GameContext &ctx, const InputState &input) const;
};
