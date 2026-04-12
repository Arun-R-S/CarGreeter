#include "audio_engine.h"

#include <driver/i2s.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "event_bus.h"
#include "file_manager.h"
#include "logger.h"

namespace {

TaskHandle_t g_audioTask = nullptr;
volatile bool g_playRequested = false;

constexpr int kI2sPort = I2S_NUM_0;
constexpr int kBclkPin = 26;
constexpr int kLrckPin = 25;
constexpr int kDataOutPin = 22;

void configureI2s() {
  const i2s_config_t config = {
      .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 44100,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = 0,
      .dma_buf_count = 6,
      .dma_buf_len = 256,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0,
  };

  const i2s_pin_config_t pins = {
      .bck_io_num = kBclkPin,
      .ws_io_num = kLrckPin,
      .data_out_num = kDataOutPin,
      .data_in_num = I2S_PIN_NO_CHANGE,
  };

  (void)i2s_driver_install(static_cast<i2s_port_t>(kI2sPort), &config, 0, nullptr);
  (void)i2s_set_pin(static_cast<i2s_port_t>(kI2sPort), &pins);
  (void)i2s_zero_dma_buffer(static_cast<i2s_port_t>(kI2sPort));
}

void audioTask(void*) {
  configureI2s();
  logInfo("AUDIO", "Audio task ready");

  for (;;) {
    if (!g_playRequested) {
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }
    g_playRequested = false;

    File f = fileManagerOpenAudioRead();
    if (!f) {
      logWarn("AUDIO", "No audio file to play");
      continue;
    }

    logInfo("AUDIO", "Playback started");

    uint8_t buf[1024];
    while (true) {
      const size_t n = f.read(buf, sizeof(buf));
      if (n == 0) {
        break;
      }
      size_t written = 0;
      (void)i2s_write(static_cast<i2s_port_t>(kI2sPort), buf, n, &written, portMAX_DELAY);
    }
    f.close();

    logInfo("AUDIO", "Playback finished");
  }
}

void onPlay(const Event&, void*) { g_playRequested = true; }

}

void audioEngineInit() { (void)eventBusRegisterHandler(EVENT_PLAY, onPlay, nullptr); }

void audioEngineStartTask(UBaseType_t priority, uint32_t stackWords) {
  if (g_audioTask != nullptr) {
    return;
  }
  const BaseType_t ok = xTaskCreate(audioTask, "audio", stackWords, nullptr, priority, &g_audioTask);
  if (ok != pdPASS) {
    g_audioTask = nullptr;
    logError("AUDIO", "Failed to start audio task");
  }
}
