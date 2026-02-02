#pragma once

/*
https://market.yandex.ru/card/svetodiodnyy-torsher-rgb-s-povorotnoy-lampoy-9617/102618658595?showOriginalKmEmptyOffer=1&ogV=-11
  -----
*/

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/light/light_traits.h"
#include <esp_gattc_api.h>

namespace esphome {
namespace istar {

class IStarOutput : public light::LightOutput, public ble_client::BLEClientNode, public Component  {
 public:
  IStarOutput();

  // LightOutput methods
  light::LightTraits get_traits() override;
  void setup_state(light::LightState *state) override { this->light_state_ = state; }
  void write_state(light::LightState *state) override;

  // Component methods
  void setup() override{};
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return esphome::setup_priority::DATA; }

  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

 protected:
  uint16_t read_handle_;
  uint16_t write_handle_;
  esp32_ble_tracker::ESPBTUUID service_read_uuid_;
  esp32_ble_tracker::ESPBTUUID service_write_uuid_;
  esp32_ble_tracker::ESPBTUUID sensors_write_characteristic_uuid_;
  esp32_ble_tracker::ESPBTUUID sensors_read_characteristic_uuid_;

  bool last_binary_{false};
  float last_red_{0};
  float last_green_{0};
  float last_blue_{0};
  float last_cold_white_{0};
  float last_warm_white_{0};
  float last_brightness_{0};

  light::LightState *light_state_{nullptr};

  void publish_state_(const bool is_on, const uint8_t brightness);
  void write_ble_(uint8_t * req, size_t sz);
};

}  // namespace istar
}  // namespace esphome
