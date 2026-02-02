import esphome.codegen as cg
from esphome.components import ble_client, light
import esphome.config_validation as cv
from esphome.const import CONF_OUTPUT_ID


DEPENDENCIES = ["ble_client", "light"]

istar_ns = cg.esphome_ns.namespace("istar")
IStarOutput = istar_ns.class_(
    "IStarOutput", cg.Component, ble_client.BLEClientNode, light.LightOutput
)

CONFIG_SCHEMA = (
    light.RGB_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(IStarOutput),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(ble_client.BLE_CLIENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
    await light.register_light(var, config)
