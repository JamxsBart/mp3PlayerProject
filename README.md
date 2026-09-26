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

- ESP32 board
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

1. Install Arduino IDE or another ESP32-compatible environment.
2. Add ESP32 board support.
3. Install these libraries:
   - `TFT_eSPI`
   - `Audio_nopsram`
   - `SD`
   - `SPI`
4. Configure `TFT_eSPI` for your display.
5. Open `src/mp3PlayerMain.ino`.
6. Select the correct ESP32 board and port.
7. Upload the sketch.
8. Insert the SD card and open the serial monitor at `115200`.

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
└── src/
    └── mp3PlayerMain.ino
```

## License

No license has been added yet.
