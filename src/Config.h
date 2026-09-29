#pragma once
#include <Arduino.h>
namespace Config {
constexpr uint8_t SD_SPI_SCK=14, SD_SPI_MOSI=13, SD_SPI_MISO=19, SD_SPI_CS=27;
constexpr uint8_t I2S_BCLK=26, I2S_LRC=25, I2S_DOUT=22;
constexpr uint8_t ENC_CLK=17, ENC_DT=33, ENC_SW=21;
constexpr uint16_t MAX_TRACKS=300, MAX_PATH_LENGTH=96;
constexpr uint8_t MIN_VOLUME=0, MAX_VOLUME=21, DEFAULT_VOLUME=15;
constexpr uint32_t SD_FREQUENCY=8000000, CLICK_TIMEOUT_MS=400, LONG_PRESS_MS=1000, EOF_GUARD_MS=2000;
constexpr uint16_t SCREEN_WIDTH=320, SCREEN_HEIGHT=240;
}
