# Setup And Upload

## Folder Layout

Arduino IDE expects the sketch file to match the folder name:

- Folder: `Tinycade`
- Sketch: [Tinycade.ino](../Tinycade.ino)

Markdown files and the `docs` folder can remain inside the sketch folder. They are not uploaded to the ESP32.

## Required Libraries

Install these in the Arduino IDE:

- ESP32 board support
- `TFT_eSPI`
- `SPI`

## Hardware Assumptions

Current shared pin assumptions from [GameTypes.h](../GameTypes.h):

- Display size: `135x240`
- `BTN_FLAP = 35`
- `BTN_ACTION = 0`
- `DISPLAY_BACKLIGHT_PIN = 4`

## TFT_eSPI Setup

Tinycade depends on your local `TFT_eSPI` configuration. Make sure your configured driver, pins, and rotation match the physical display before upload.

## Upload Workflow

1. Open [Tinycade.ino](../Tinycade.ino).
2. Select the correct ESP32 board.
3. Select the correct COM/serial port.
4. Verify the `TFT_eSPI` setup.
5. Compile.
6. Upload.

## Testing Notes

When reporting results from the ESP32, it helps to include:

- Which game you launched
- Which button you pressed
- Whether the bug happened at start, during play, or on restart
- Whether the issue was visual, input-related, or gameplay-related

## Restart Notes

- Most game-over screens now require a short press of both buttons to retry.
- Dash, Runner, Dino, and Punch include a short grace window after retry so the player is not dropped directly into immediate danger.
