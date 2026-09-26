# ESP32 MP3 Player

A simple SD-card-based MP3/WAV player built with an ESP32, TFT display, I2S audio output, and a rotary encoder. The player scans a `/library` directory on the SD card, starts with a random track, displays the current filename, and lets you pause or skip playback with the encoder button.

## Features

- Plays `.mp3` and `.wav` files from an SD card
- Automatically scans `/library` at startup
- Randomly selects the first track
- Automatically chooses another random track when playback ends
- TFT display showing the current track and playback state
- Single-click to pause or resume
- Double-click to skip to another random track
- Supports up to 300 tracks

## Hardware

- ESP32 development board
- TFT display supported by [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
- MicroSD card module
- I2S DAC, amplifier, or audio module
- Rotary encoder with push button
- Speaker or headphones connected to the audio output

## Pin Configuration

The current sketch uses these pins:

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

> Confirm the pinout for your specific ESP32 board and modules before wiring. The TFT display pins are configured separately through `TFT_eSPI`.

## SD Card Layout

Format the card as a compatible FAT filesystem and create a directory named `library` in its root:

```text
SD card/
└── library/
    ├── track-01.mp3
    ├── track-02.wav
    └── another-song.mp3
```

Only files with `.mp3` and `.wav` extensions are loaded. Extension matching is case-insensitive.

## Software Setup

1. Install the Arduino IDE or another ESP32-compatible development environment.
2. Install/configure the ESP32 board support package.
3. Install the required libraries:
   - `TFT_eSPI`
   - `Audio_nopsram` (the audio library providing the `Audio` class used by the sketch)
   - `SD`
   - `SPI`
4. Configure `TFT_eSPI` for your display and wiring.
5. Open `src/mp3PlayerMain.ino`.
6. Select the correct ESP32 board and serial port.
7. Upload the sketch.
8. Insert the prepared SD card and open the serial monitor at **115200 baud** for diagnostic messages.

## Controls

| Action | Result |
| --- | --- |
| Single press | Pause or resume the current track |
| Double press | Skip to another random track |
| Track finishes | Automatically play another random track |

The encoder rotation pins are currently defined for future controls; playback control is handled by the encoder push button.

## Configuration

Hardware pins and player limits are defined near the top of `src/mp3PlayerMain.ino`. Adjust these values for your build:

- SD card SPI pins
- I2S audio pins
- Rotary encoder button pin
- `MAX_TRACKS` — maximum number of tracks, currently `300`
- `MAX_PATH` — maximum stored path length, currently `96`

The default audio volume is set with `audio.setVolume(15)`.

## Troubleshooting

### `SD card failed to load`

- Check the SD card wiring and chip-select pin.
- Confirm the card is formatted correctly.
- Make sure the SD card module uses compatible logic levels.
- Verify that GPIO 14, 13, 19, and 27 match your wiring.

### `No songs found`

- Confirm that the directory is named exactly `/library`.
- Check that the files end in `.mp3` or `.wav`.
- Ensure the files are directly inside `/library`, rather than in nested folders.

### No audio output

- Verify the I2S BCLK, LRC/WS, and DOUT connections.
- Confirm that the selected audio module supports the file format.
- Check the amplifier, speaker, and power supply.
- Use the serial monitor to confirm that tracks are being opened.

## Project Structure

```text
.
├── README.md
└── src/
    └── mp3PlayerMain.ino
```

## License

No license has been specified for this project yet. Add a license if you plan to distribute or reuse the code.