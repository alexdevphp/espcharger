import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import CONF_ID

from . import ESPChargerComponent, espcharger_ns

DEPENDENCIES = ["espcharger"]

CONF_ENABLE_EDIT = "enable_edit"
CONF_GET_TELEMETRY = "get_telemetry"

ESPChargerEnableEditButton = espcharger_ns.class_("ESPChargerEnableEditButton", button.Button)
ESPChargerGetTelemetryButton = espcharger_ns.class_("ESPChargerGetTelemetryButton", button.Button)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(ESPChargerComponent),
        cv.Optional(CONF_ENABLE_EDIT): button.button_schema(ESPChargerEnableEditButton),
        cv.Optional(CONF_GET_TELEMETRY): button.button_schema(ESPChargerGetTelemetryButton),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])

    if enable_edit_config := config.get(CONF_ENABLE_EDIT):
        var = await button.new_button(enable_edit_config)
        await cg.register_parented(var, config[CONF_ID])
        cg.add(parent.set_enable_edit_button(var))

    if get_tlm_config := config.get(CONF_GET_TELEMETRY):
        var = await button.new_button(get_tlm_config)
        await cg.register_parented(var, config[CONF_ID])
        cg.add(parent.set_get_telemetry_button(var))
