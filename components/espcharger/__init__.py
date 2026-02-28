import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@esphome"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "number", "switch", "button"]

espcharger_ns = cg.esphome_ns.namespace("espcharger")
ESPChargerComponent = espcharger_ns.class_(
    "ESPChargerComponent", cg.PollingComponent, uart.UARTDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ESPChargerComponent),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
