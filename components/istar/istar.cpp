#include "istar.h"
#include <string>
#include "hex2bin.h"

#define SERVICE_UUID_W "0000ffd5-0000-1000-8000-00805f9b34fb"
#define SERVICE_UUID_R "0000ffd0-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID_W "0000ffd9-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID_R "0000ffd4-0000-1000-8000-00805f9b34fb"

namespace esphome {
namespace istar {

static const char *const TAG = "istar";

IStarOutput::IStarOutput() :
    service_read_uuid_(esp32_ble_tracker::ESPBTUUID::from_raw(SERVICE_UUID_R)),
    service_write_uuid_(esp32_ble_tracker::ESPBTUUID::from_raw(SERVICE_UUID_W)),
    sensors_write_characteristic_uuid_(esp32_ble_tracker::ESPBTUUID::from_raw(CHARACTERISTIC_UUID_W)),
    sensors_read_characteristic_uuid_(esp32_ble_tracker::ESPBTUUID::from_raw(CHARACTERISTIC_UUID_R))
    {}

// Set the device's traits
light::LightTraits IStarOutput::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::RGB, light::ColorMode::COLD_WARM_WHITE});
  traits.set_min_mireds(155);
  traits.set_max_mireds(380);
  return traits;
}

void IStarOutput::publish_state_(const bool is_on, const uint8_t brightness) {
  if (light_state_) {
    ESP_LOGV(TAG, "Publishing new state: %s, brightness=%d", ONOFF(is_on), brightness);
    auto call = light_state_->make_call();
    call.set_state(is_on);
    call.set_brightness((float) brightness / 100.0f);
    call.perform();
  }
}

struct CmdOnOff {  uint8_t b0=0xc1, onoff=0, b2=0x1c; };
struct CmdSet   {  uint8_t b0=0x56, r=0, g=0, b=0, mode=0x0f, ww=0, cw=0, brightness=0, b8=0x00, b9=0xaa; };

void IStarOutput::write_state(light::LightState *state) {
  bool binary;
  float red;
  float green;
  float blue;
  float cold_white;
  float warm_white;
  float brightness;

  // Fill our variables with the device's current state
  state->current_values_as_binary(&binary);
  state->current_values_as_brightness(&brightness);
  state->current_values_as_rgbww(&red, &green, &blue, &cold_white, &warm_white, true);
  esphome::light::ColorMode mode = state->current_values.get_color_mode();

  binary = binary || brightness > 0;

  if (binary != last_binary_)
  {
    last_binary_ = binary;
    CmdOnOff cmd;
    cmd.onoff = binary ? 0x23 : 0x24;
    write_ble_((uint8_t *)&cmd, sizeof(cmd));
  }
  else
  {
    CmdSet cmd;
    cmd.r = red * 255;
    cmd.g = green * 255;
    cmd.b = blue * 255;
    cmd.mode = (mode == light::ColorMode::RGB) ? 0xf0 : 0x0f;
    cmd.cw = cold_white * 255;
    cmd.ww = warm_white * 255;
    cmd.brightness = brightness * 100;

    if (last_red_ != cmd.r || last_green_ != cmd.g || last_blue_ != cmd.b || last_cold_white_ != cmd.cw || last_warm_white_ != cmd.ww || last_brightness_ != cmd.brightness) {
      write_ble_((uint8_t *)&cmd, sizeof(cmd));

      last_red_ = cmd.r;
      last_green_ = cmd.g;
      last_blue_ = cmd.b;
      last_cold_white_ = cmd.cw;
      last_warm_white_ = cmd.ww;
      last_brightness_ = cmd.brightness;
    }
  }

  //ESP_LOGD(TAG, "write_state(red = %f, green = %f, blue = %f, cold_white = %f, warm_white = %f) brightness=%f, binary=%d", red, green, blue, cold_white, warm_white, brightness, binary);
}

void IStarOutput::write_ble_(uint8_t * req, size_t sz) {
  ESP_LOGD(TAG, "ble >> %s\n", hextools::bin2hex(req, sz).c_str(), ' ');
  auto status = esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(),
                                               this->write_handle_, sz, req,
                                               ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
  if (status) {
    ESP_LOGW(TAG, "Error sending write request for sensor, status=%d", status);
  }
}

void IStarOutput::dump_config() {
  ESP_LOGCONFIG(TAG,
                "iStar: '%s'\n",
                this->light_state_ ? this->light_state_->get_name().c_str() : "");
}

void IStarOutput::loop() {
}

void IStarOutput::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                          esp_ble_gattc_cb_param_t *param) {

  //ESP_LOGD(TAG, "gattc_event_handler() %d", event);
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      if (param->open.status == ESP_GATT_OK) {
        ESP_LOGI(TAG, "Connected successfully!");
      }
      break;
    }

    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGW(TAG, "Disconnected!");
      break;
    }

    case ESP_GATTC_SEARCH_CMPL_EVT: {
      this->read_handle_ = 0;
      auto *chr = this->parent()->get_characteristic(service_read_uuid_, sensors_read_characteristic_uuid_);
      if (chr == nullptr) {
        ESP_LOGW(TAG, "No sensor read characteristic found at service %s char %s", service_read_uuid_.to_string().c_str(),
                 sensors_read_characteristic_uuid_.to_string().c_str());
        break;
      }
      this->read_handle_ = chr->handle;

      auto *write_chr = this->parent()->get_characteristic(service_write_uuid_, sensors_write_characteristic_uuid_);
      if (write_chr == nullptr) {
        ESP_LOGW(TAG, "No sensor write characteristic found at service %s char %s", service_write_uuid_.to_string().c_str(),
                 sensors_write_characteristic_uuid_.to_string().c_str());
        break;
      }
      this->write_handle_ = write_chr->handle;

      this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;

      //write_query_message_();

      //request_read_values_();
      break;
    }
    default:
      break;
  }
}


}  // namespace istar
}  // namespace esphome
