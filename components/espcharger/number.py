import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_CURRENT, CONF_ID, CONF_VOLTAGE

from . import ESPChargerComponent, espcharger_ns

DEPENDENCIES = ["espcharger"]

ESPChargerVoltageNumber = espcharger_ns.class_("ESPChargerVoltageNumber", number.Number)
ESPChargerCurrentNumber = espcharger_ns.class_("ESPChargerCurrentNumber", number.Number)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(ESPChargerComponent),
        cv.Optional(CONF_VOLTAGE): number.number_schema(
            ESPChargerVoltageNumber,
            min_value=10,
            max_value=93,
            step=1,
        ),
        cv.Optional(CONF_CURRENT): number.number_schema(
            ESPChargerCurrentNumber,
            min_value=1,
            max_value=20,
            step=1,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])

    if voltage_config := config.get(CONF_VOLTAGE):
        var = await number.new_number(voltage_config)
        await cg.register_parented(var, config[CONF_ID])
        cg.add(parent.set_voltage_number(var))

    if current_config := config.get(CONF_CURRENT):
        var = await number.new_number(current_config)
        await cg.register_parented(var, config[CONF_ID])
        cg.add(parent.set_current_number(var))
