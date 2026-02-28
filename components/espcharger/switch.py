import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID

from . import ESPChargerComponent, espcharger_ns

DEPENDENCIES = ["espcharger"]

CONF_CHARGING = "charging"

ESPChargerChargingSwitch = espcharger_ns.class_("ESPChargerChargingSwitch", switch.Switch)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(ESPChargerComponent),
        cv.Optional(CONF_CHARGING): switch.switch_schema(ESPChargerChargingSwitch),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])

    if charging_config := config.get(CONF_CHARGING):
        var = await switch.new_switch(charging_config)
        await cg.register_parented(var, config[CONF_ID])
        cg.add(parent.set_charging_switch(var))
