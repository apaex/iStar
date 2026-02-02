# Connecting the iStar ML-CTD01 (LedSwet) Smart BLE LED Floor Lamp to the Home Assistant via ESPHome and ESP32

https://market.yandex.ru/card/svetodiodnyy-torsher-rgb-s-povorotnoy-lampoy-9617/102618658595?showOriginalKmEmptyOffer=1&ogV=-11

### Add it to yaml:

```yaml
esp32:
  board: esp32dev
  framework:
    type: arduino

external_components:
  - source: github://apaex/iStar

esp32_ble_tracker:

ble_client:
  - id: ble_istar1
    mac_address: 2B:80:03:D9:3E:8E

light:
  - platform: istar
    ble_client_id: ble_istar1
    name: iStar Light

sensor:
  - platform: ble_rssi
    mac_address: 2B:80:03:D9:3E:8E
    name: iStar RSSI value
```
