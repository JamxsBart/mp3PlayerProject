/*
 * ESP32 MP3 Player
 * Refactored v2
 *
 * Hardware configuration is intentionally unchanged from v1:
 * SD:       SCK 14, MOSI 13, MISO 19, CS 27
 * I2S:      BCLK 26, LRC 25, DOUT 22
 * Encoder:  CLK 17, DT 33, SW 21
 */

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include "Audio_nopsram.h"

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

namespace Config {
constexpr uint8_t SD_SCK  = 14;
constexpr uint8_t SD_MOSI = 13;
constexpr uint8_t SD_MISO = 19;
constexpr uint8_t SD_CS   = 27;

constexpr uint8_t I2S_BCLK = 26;
constexpr uint8_t I2S_LRC  = 25;
constexpr uint8_t I2S_DOUT = 22;

constexpr uint8_t ENCODER_CLK = 17;
constexpr uint8_t ENCODER_DT  = 33;
constexpr uint8_t ENCODER_SW  = 21;

constexpr uint16_t MAX_TRACKS = 300;
constexpr uint16_t MAX_PATH_LENGTH = 96;

constexpr uint8_t MIN_VOLUME = 0;
constexpr uint8_t MAX_VOLUME = 21;
constexpr uint8_t DEFAULT_VOLUME = 15;

constexpr uint32_t SD_FREQUENCY = 8000000;
constexpr uint32_t CLICK_TIMEOUT_MS = 400;
constexpr uint32_t LONG_PRESS_MS = 1000;
constexpr uint32_t EOF_GUARD_MS = 2000;

constexpr uint16_t SCREEN_WIDTH = 320;
constexpr uint16_t SCREEN_HEIGHT = 240;
}

// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------

enum class PlayerState {
    Playing,
    Paused,
    Menu,
    Error
};

struct Track {
    char path[Config::MAX_PATH_LENGTH];
};

// -----------------------------------------------------------------------------
// Playlist
// -----------------------------------------------------------------------------

class Playlist {
public:
    bool load()
    {
        trackCount = 0;

        File directory = SD.open("/library");

        if (!directory || !directory.isDirectory()) {
            if (directory) {
                directory.close();
            }
            return false;
        }

        while (trackCount < Config::MAX_TRACKS) {
            File entry = directory.openNextFile();

            if (!entry) {
                break;
            }

            if (!entry.isDirectory() && isAudioFile(entry.name())) {
                addTrack(entry.name());
            }

            entry.close();
        }

        directory.close();

        Serial.print("Found ");
        Serial.print(trackCount);
        Serial.println(" songs");

        return true;
    }

    uint16_t size() const
    {
        return trackCount;
    }

    bool empty() const
    {
        return trackCount == 0;
    }

    const Track* get(int index) const
    {
        if (index < 0 || index >= trackCount) {
            return nullptr;
        }

        return &tracks[index];
    }

    int randomIndexExcept(int currentIndex) const
    {
        if (trackCount == 0) {
            return -1;
        }

        if (trackCount == 1) {
            return 0;
        }

        int index;

        do {
            index = random(0, trackCount);
        } while (index == currentIndex);

        return index;
    }

private:
    Track tracks[Config::MAX_TRACKS];
    uint16_t trackCount = 0;

    static bool isAudioFile(const char* filename)
    {
        const size_t length = strlen(filename);

        if (length < 5) {
            return false;
        }

        const char* extension = filename + length - 4;

        return strcasecmp(extension, ".mp3") == 0 ||
               strcasecmp(extension, ".wav") == 0;
    }

    void addTrack(const char* filename)
    {
        if (trackCount >= Config::MAX_TRACKS) {
            return;
        }

        if (filename[0] == '/') {
            snprintf(
                tracks[trackCount].path,
                Config::MAX_PATH_LENGTH,
                "%s",
                filename
            );
        } else {
            snprintf(
                tracks[trackCount].path,
                Config::MAX_PATH_LENGTH,
                "/library/%s",
                filename
            );
        }

        ++trackCount;
    }
};

// -----------------------------------------------------------------------------
// Display
// -----------------------------------------------------------------------------

class Display {
public:
    void begin()
    {
        tft.init();
        tft.setRotation(1);
        showLoading();
    }

    void showLoading()
    {
        clear();
        setText(TFT_WHITE, 2);
        tft.setCursor(10, 10);
        tft.println("Loading...");
    }

    void showError(const char* message)
    {
        clear();

        setText(TFT_RED, 2);
        tft.setCursor(10, 10);
        tft.println("ERROR");

        setText(TFT_WHITE, 2);
        tft.setCursor(10, 50);
        tft.println(message);
    }

    void showNoTracks()
    {
        clear();

        setText(TFT_YELLOW, 2);
        tft.setCursor(10, 10);
        tft.println("No songs found");

        setText(TFT_WHITE, 1);
        tft.setCursor(10, 50);
        tft.println("Add MP3/WAV files to");
        tft.setCursor(10, 65);
        tft.println("/library");
    }

    void showNowPlaying(
        const char* path,
        int trackNumber,
        int totalTracks,
        PlayerState state,
        int volume
    )
    {
        clear();

        setText(TFT_CYAN, 2);
        tft.setCursor(10, 10);
        tft.println("NOW PLAYING");

        drawTrackName(path);

        const bool paused = state == PlayerState::Paused;

        setText(paused ? TFT_ORANGE : TFT_GREEN, 2);
        tft.setCursor(10, 160);
        tft.println(paused ? "PAUSED" : "PLAYING");

        setText(TFT_DARKGREY, 1);
        tft.setCursor(10, 210);
        tft.print("Song ");
        tft.print(trackNumber);
        tft.print(" of ");
        tft.print(totalTracks);

        drawVolume(volume);
    }

    void showMenu()
    {
        clear();

        setText(TFT_WHITE, 2);
        tft.setCursor(10, 10);
        tft.println("MENU");

        setText(TFT_YELLOW, 2);
        tft.setCursor(10, 60);
        tft.println("Return to Music");

        setText(TFT_DARKGREY, 1);
        tft.setCursor(10, 220);
        tft.println("Press button to return");
    }

private:
    TFT_eSPI tft;

    void clear()
    {
        tft.fillScreen(TFT_BLACK);
    }

    void setText(uint16_t color, uint8_t size)
    {
        tft.setTextColor(color, TFT_BLACK);
        tft.setTextSize(size);
    }

    void drawVolume(int volume)
    {
        tft.fillRect(0, 230, Config::SCREEN_WIDTH, 10, TFT_BLACK);

        setText(TFT_WHITE, 1);
        tft.setCursor(10, 230);
        tft.print("Volume: ");
        tft.print(volume);
    }

    void drawTrackName(const char* path)
    {
        const char* slash = strrchr(path, '/');
        const char* baseName = slash ? slash + 1 : path;

        char name[80];
        strncpy(name, baseName, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';

        char* extension = strrchr(name, '.');

        if (extension) {
            *extension = '\0';
        }

        const int length = strlen(name);

        setText(TFT_WHITE, 2);

        if (length <= 22) {
            int x = (Config::SCREEN_WIDTH - length * 12) / 2;

            if (x < 0) {
                x = 0;
            }

            tft.setCursor(x, 100);
            tft.println(name);
            return;
        }

        int split = length / 2;

        for (int offset = 0; offset < 10; ++offset) {
            if (split + offset < length && name[split + offset] == ' ') {
                split += offset;
                break;
            }

            if (split - offset > 0 && name[split - offset] == ' ') {
                split -= offset;
                break;
            }
        }

        char firstLine[80];
        char secondLine[80];

        strncpy(firstLine, name, split);
        firstLine[split] = '\0';

        const int secondStart = (split < length && name[split] == ' ')
            ? split + 1
            : split;

        strncpy(
            secondLine,
            name + secondStart,
            sizeof(secondLine) - 1
        );

        secondLine[sizeof(secondLine) - 1] = '\0';

        tft.setCursor(10, 80);
        tft.println(firstLine);

        tft.setCursor(10, 110);
        tft.println(secondLine);
    }
};

// -----------------------------------------------------------------------------
// Rotary encoder
// -----------------------------------------------------------------------------

class Encoder {
public:
    void begin()
    {
        pinMode(Config::ENCODER_CLK, INPUT_PULLUP);
        pinMode(Config::ENCODER_DT, INPUT_PULLUP);

        previousState =
            (digitalRead(Config::ENCODER_CLK) << 1) |
            digitalRead(Config::ENCODER_DT);

        counter = 0;
        lastCounter = 0;

        attachInterrupt(
            digitalPinToInterrupt(Config::ENCODER_CLK),
            isr,
            CHANGE
        );

        attachInterrupt(
            digitalPinToInterrupt(Config::ENCODER_DT),
            isr,
            CHANGE
        );
    }

    int consumeClicks()
    {
        noInterrupts();
        const int currentCounter = counter;
        interrupts();

        const int steps = currentCounter - lastCounter;
        const int clicks = steps / 4;

        if (clicks != 0) {
            lastCounter += clicks * 4;
        }

        return clicks;
    }

private:
    static volatile int counter;
    static volatile uint8_t previousState;

    int lastCounter = 0;

    static void IRAM_ATTR isr()
    {
        const uint8_t currentState =
            (digitalRead(Config::ENCODER_CLK) << 1) |
            digitalRead(Config::ENCODER_DT);

        const uint8_t transition =
            (previousState << 2) | currentState;

        switch (transition) {
            case 0b0001:
            case 0b0111:
            case 0b1110:
            case 0b1000:
                --counter;
                break;

            case 0b0010:
            case 0b1011:
            case 0b1101:
            case 0b0100:
                ++counter;
                break;

            default:
                break;
        }

        previousState = currentState;
    }
};

volatile int Encoder::counter = 0;
volatile uint8_t Encoder::previousState = 0;

// -----------------------------------------------------------------------------
// Button input
// -----------------------------------------------------------------------------

enum class ButtonEvent {
    None,
    SingleClick,
    DoubleClick,
    LongPress
};

class Button {
public:
    void begin()
    {
        pinMode(Config::ENCODER_SW, INPUT_PULLUP);
    }

    ButtonEvent update()
    {
        const bool pressed = digitalRead(Config::ENCODER_SW) == LOW;
        const uint32_t now = millis();

        if (pressed && !lastPressed) {
            pressStart = now;
            longPressHandled = false;

            if (clickCount == 0) {
                firstClick = now;
            }

            ++clickCount;
        }

        if (pressed &&
            !longPressHandled &&
            now - pressStart >= Config::LONG_PRESS_MS) {

            longPressHandled = true;
            clickCount = 0;
            lastPressed = pressed;

            return ButtonEvent::LongPress;
        }

        if (!pressed && lastPressed && longPressHandled) {
            longPressHandled = false;
            lastPressed = pressed;
            return ButtonEvent::None;
        }

        lastPressed = pressed;

        if (clickCount > 0 &&
            now - firstClick >= Config::CLICK_TIMEOUT_MS) {

            const ButtonEvent event =
                clickCount == 1
                    ? ButtonEvent::SingleClick
                    : ButtonEvent::DoubleClick;

            clickCount = 0;
            return event;
        }

        return ButtonEvent::None;
    }

private:
    bool lastPressed = false;
    bool longPressHandled = false;
    uint8_t clickCount = 0;

    uint32_t firstClick = 0;
    uint32_t pressStart = 0;
};

// -----------------------------------------------------------------------------
// Audio player
// -----------------------------------------------------------------------------

class AudioPlayer {
public:
    void begin()
    {
        audio.setPinout(
            Config::I2S_BCLK,
            Config::I2S_LRC,
            Config::I2S_DOUT
        );

        audio.setVolume(volume);

        Serial.println("Audio initialised");
    }

    void update()
    {
        audio.loop();
    }

    void play(const char* path)
    {
        paused = false;
        trackStarted = millis();

        noInterrupts();
        trackEnded = false;
        interrupts();

        audio.connecttoFS(SD, path);

        Serial.print("Playing: ");
        Serial.println(path);
    }

    void togglePause()
    {
        audio.pauseResume();
        paused = !paused;
    }

    void setVolume(int newVolume)
    {
        volume = constrain(
            newVolume,
            Config::MIN_VOLUME,
            Config::MAX_VOLUME
        );

        audio.setVolume(volume);
    }

    int getVolume() const
    {
        return volume;
    }

    bool isPaused() const
    {
        return paused;
    }

    bool consumeTrackEnded()
    {
        noInterrupts();
        const bool ended = trackEnded;
        trackEnded = false;
        interrupts();

        return ended;
    }

    void onEndOfFile()
    {
        if (millis() - trackStarted < Config::EOF_GUARD_MS) {
            Serial.println("(ignoring spurious EOF)");
            return;
        }

        trackEnded = true;
    }

private:
    Audio audio;

    int volume = Config::DEFAULT_VOLUME;
    bool paused = false;

    volatile bool trackEnded = false;
    uint32_t trackStarted = 0;

    friend void audio_eof_mp3(const char* info);
};

// -----------------------------------------------------------------------------
// Player application
// -----------------------------------------------------------------------------

class Player {
public:
    void begin()
    {
        Serial.begin(115200);
        delay(500);

        Serial.println();
        Serial.println("--- ESP32 MP3 Player v2 ---");

        display.begin();

        spi.begin(
            Config::SD_SCK,
            Config::SD_MISO,
            Config::SD_MOSI,
            Config::SD_CS
        );

        if (!SD.begin(Config::SD_CS, spi, Config::SD_FREQUENCY)) {
            Serial.println("SD initialisation failed");
            state = PlayerState::Error;
            display.showError("SD card failed");
            return;
        }

        Serial.println("SD working");

        audio.begin();

        if (!playlist.load()) {
            Serial.println("Could not open /library");
            state = PlayerState::Error;
            display.showError("Library not found");
            return;
        }

        if (playlist.empty()) {
            state = PlayerState::Error;
            display.showNoTracks();
            return;
        }

        encoder.begin();
        button.begin();

        randomSeed(esp_random());

        state = PlayerState::Playing;
        ready = true;

        playRandomTrack();
    }

    void update()
    {
        if (!ready) {
            return;
        }

        audio.update();

        handleTrackEnd();
        handleEncoder();
        handleButton(button.update());
    }

    void onAudioEnd()
    {
        audio.onEndOfFile();
    }

private:
    SPIClass spi = SPIClass(HSPI);

    Display display;
    Playlist playlist;
    Encoder encoder;
    Button button;
    AudioPlayer audio;

    PlayerState state = PlayerState::Error;

    int currentTrack = -1;
    bool ready = false;

    void playTrack(int index)
    {
        const Track* track = playlist.get(index);

        if (!track) {
            return;
        }

        currentTrack = index;
        state = PlayerState::Playing;

        audio.play(track->path);
        refreshDisplay();
    }

    void playRandomTrack()
    {
        const int nextTrack =
            playlist.randomIndexExcept(currentTrack);

        if (nextTrack >= 0) {
            playTrack(nextTrack);
        }
    }

    void handleTrackEnd()
    {
        if (audio.consumeTrackEnded()) {
            playRandomTrack();
        }
    }

    void handleEncoder()
    {
        if (state == PlayerState::Menu) {
            return;
        }

        const int clicks = encoder.consumeClicks();

        if (clicks == 0) {
            return;
        }

        audio.setVolume(audio.getVolume() + clicks);

        Serial.print("Volume: ");
        Serial.println(audio.getVolume());

        refreshDisplay();
    }

    void handleButton(ButtonEvent event)
    {
        switch (event) {
            case ButtonEvent::SingleClick:
                handleSingleClick();
                break;

            case ButtonEvent::DoubleClick:
                if (state != PlayerState::Menu) {
                    playRandomTrack();
                    Serial.println("Skipped to random track");
                }
                break;

            case ButtonEvent::LongPress:
                state = PlayerState::Menu;
                display.showMenu();
                break;

            case ButtonEvent::None:
                break;
        }
    }

    void handleSingleClick()
    {
        if (state == PlayerState::Menu) {
            state = audio.isPaused()
                ? PlayerState::Paused
                : PlayerState::Playing;

            refreshDisplay();
            return;
        }

        audio.togglePause();

        state = audio.isPaused()
            ? PlayerState::Paused
            : PlayerState::Playing;

        Serial.println(audio.isPaused() ? "Paused" : "Playing");

        refreshDisplay();
    }

    void refreshDisplay()
    {
        const Track* track = playlist.get(currentTrack);

        if (!track) {
            return;
        }

        display.showNowPlaying(
            track->path,
            currentTrack + 1,
            playlist.size(),
            state,
            audio.getVolume()
        );
    }
};

Player player;

// Audio_nopsram callback.
// Keep this function at global scope because the library looks for this
// exact callback name.
void audio_eof_mp3(const char* info)
{
    (void)info;
    player.onAudioEnd();
}

// -----------------------------------------------------------------------------
// Arduino entry points
// -----------------------------------------------------------------------------

void setup()
{
    player.begin();
}

void loop()
{
    player.update();
}
