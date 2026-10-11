# ESP32 MP3 Player

A small ESP32 project that plays MP3 and WAV files from an SD card and shows the current track on a TFT display.

## What it does

- Reads audio files from `/library` on the SD card
- Supports `.mp3` and `.wav`
- Picks a random track at startup
- Plays another random track when the current one ends
- Shows the track name and playback state on the TFT display
- Uses a rotary encoder button to pause/resume or skip tracks

## Hardware

- ESP32 Dev Module without PSRAM
- TFT display compatible with `TFT_eSPI`
- MicroSD card module
- I2S audio output module or DAC
- Rotary encoder with push button
- Speaker or headphones

## Pin setup

| Function | GPIO |
| --- | ---: |
| SD card SCK | 14 |
| SD card MOSI | 13 |
| SD card MISO | 19 |
| SD card CS | 27 |
| I2S BCLK | 26 |
| I2S LRC/WS | 25 |
| I2S DOUT | 22 |
| Encoder CLK | 17 |
| Encoder DT | 33 |
| Encoder button | 21 |

> Double-check these pins against your board and modules before wiring.

## SD card layout

Format the SD card as FAT and create a `library` folder at the root:

```text
SD card/
└── library/
    ├── track-01.mp3
    ├── track-02.wav
    └── another-song.mp3
```

Only files ending in `.mp3` or `.wav` are used.

## Software

This project uses these versions:

- Arduino IDE `2.3.8`
- Espressif ESP32 board package `3.3.7`
- `TFT_eSPI` version `2.5.43`
- [`Audio_nopsram`](https://github.com/PLSousa/ESP32-audioI2S) commit `e9afa7f8f3d899a824bf46ab559e0eb8adf0b015`

`SD` and `SPI` are included with the ESP32 board package and do not need to be installed separately.

1. Install Arduino IDE `2.3.8`.
2. Install version `3.3.7` of `esp32` by Espressif Systems in the Boards Manager.
3. Install version `2.5.43` of `TFT_eSPI` in the Library Manager.
4. Download the pinned [`Audio_nopsram` ZIP](https://github.com/PLSousa/ESP32-audioI2S/archive/e9afa7f8f3d899a824bf46ab559e0eb8adf0b015.zip) and add it using **Sketch > Include Library > Add .ZIP Library**.
5. Configure `TFT_eSPI` for your display.
6. Open `mp3PlayerMain/mp3PlayerMain.ino`.
7. Select `ESP32 Dev Module` as the board.
8. Set PSRAM to `Disabled`, CPU frequency to `240 MHz`, and partition scheme to `Huge APP (3MB No OTA/1MB SPIFFS)`.
9. Select the correct port and upload the sketch.
10. Insert the SD card and open the serial monitor at `115200`.

The sketch uses `Audio_nopsram.h`. Do not replace it with the current `Audio.h` version of ESP32-audioI2S because it uses a different API and requires PSRAM.

## Controls

- Single press: pause/resume
- Double press: skip to another random track
- Track ends: automatically plays another random track

## Troubleshooting

### SD card failed to load

- Check wiring and CS pin
- Reformat the card
- Verify GPIO and module compatibility

### No songs found

- Make sure the folder is named `library`
- Use `.mp3` or `.wav` files
- Keep files in the root of `/library`

### No audio output

- Check I2S wiring
- Confirm the audio module supports the file type
- Verify speaker/amplifier power
- Watch the serial monitor for track loading messages

## Project files

```text
.
├── README.md
└── mp3PlayerMain/
    ├── Config.h
    ├── Controls.cpp
    ├── Controls.h
    ├── Display.cpp
    ├── Display.h
    ├── Player.cpp
    ├── Player.h
    └── mp3PlayerMain.ino
```

- `mp3PlayerMain.ino` starts and updates the project
- `Config.h` stores the pin numbers and limits
- `Controls` handles the rotary encoder and its button
- `Display` draws the player screens
- `Player` handles the SD card, playlist and audio playback

## License

No license has been added yet.
