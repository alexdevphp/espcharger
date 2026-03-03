import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_CURRENT, CONF_ID, CONF_VOLTAGE
from . import ESPChargerComponent, espcharger_ns
CONF_MAX_VALUE = "max_value"
CONF_MIN_VALUE = "min_value"
CONF_STEP = "step"

DEPENDENCIES = ["espcharger"]

ESPChargerVoltageNumber = espcharger_ns.class_("ESPChargerVoltageNumber", number.Number)
ESPChargerCurrentNumber = espcharger_ns.class_("ESPChargerCurrentNumber", number.Number)


VOLTAGE_NUMBER_SCHEMA = number.number_schema(ESPChargerVoltageNumber).extend(
    {
        cv.Optional(CONF_MAX_VALUE, default=93.0): cv.float_,
        cv.Optional(CONF_MIN_VALUE, default=10.0): cv.float_,
        cv.Optional(CONF_STEP, default=1.0): cv.float_,
    }
)

CURRENT_NUMBER_SCHEMA = number.number_schema(ESPChargerCurrentNumber).extend(
    {
        cv.Optional(CONF_MAX_VALUE, default=20.0): cv.float_,
        cv.Optional(CONF_MIN_VALUE, default=1.0): cv.float_,
        cv.Optional(CONF_STEP, default=1.0): cv.float_,
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(ESPChargerComponent),
        cv.Optional(CONF_VOLTAGE): VOLTAGE_NUMBER_SCHEMA,
        cv.Optional(CONF_CURRENT): CURRENT_NUMBER_SCHEMA,
    }
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])

    if voltage_config := config.get(CONF_VOLTAGE):
        v_conf = config[CONF_VOLTAGE]
        var = await number.new_number(voltage_config, min_value=v_conf[CONF_MIN_VALUE], max_value=v_conf[CONF_MAX_VALUE], step=v_conf[CONF_STEP])
        await cg.register_parented(var, config[CONF_ID])
        cg.add(parent.set_voltage_number(var))

    if current_config := config.get(CONF_CURRENT):
        c_conf = config[CONF_CURRENT]
        var = await number.new_number(current_config, min_value=c_conf[CONF_MIN_VALUE], max_value=c_conf[CONF_MAX_VALUE], step=c_conf[CONF_STEP])
        await cg.register_parented(var, config[CONF_ID])
        cg.add(parent.set_current_number(var))
