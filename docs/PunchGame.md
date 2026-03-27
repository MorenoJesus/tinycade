# Punch Game

## Overview

`PunchGame.h` adds a fast reaction-based fighting game inspired by the feel of One Finger Death Punch. It is simplified for the ESP32 and Tinycade control layout: enemies approach from the left and right lanes, and the player must strike the correct side at the right moment.

## Controls

- `BTN2` attacks the left lane
- `BTN1` attacks the right lane
- On intro, either button starts
- On game over, a short press of both buttons restarts the round

## Gameplay

- Enemies spawn in either the left or right lane.
- Each enemy advances toward the player over time.
- A correct strike removes the nearest enemy on that side.
- Hitting too early or striking the wrong side costs a life.
- Letting an enemy reach the player also costs a life.
- Heavy enemies appear later and award more score.
- Combo streaks add bonus points every five hits.
- Successful hits trigger a short burst/explosion effect for stronger feedback.
- After a retry, the game waits briefly before enemies begin spawning.

## State Flow

- `Intro`
- `Running`
- `GameOver`

## Design Notes

- The game uses silhouette fighters and lane-based movement to keep rendering cheap on the ESP32.
- This is inspired by the pacing and side-selection feel of One Finger Death Punch, not a direct clone.
- The module is header-only and plugs into the existing `IGame` interface.

## Files

- [PunchGame.h](../Games/PunchGame.h)
