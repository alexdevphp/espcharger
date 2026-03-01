import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_CURRENT,
    CONF_ID,
    CONF_MODE,
    CONF_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_VOLT,
)

from . import ESPChargerComponent

DEPENDENCIES = ["espcharger"]

CONF_CHARGE_COUNTER = "charge_counter"
CONF_TEMPERATURE_1 = "temperature_1"
CONF_TEMPERATURE_2 = "temperature_2"
CONF_CHARGING_STATE = "charging_state"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(ESPChargerComponent),
        cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_CURRENT): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_CHARGE_COUNTER): sensor.sensor_schema(accuracy_decimals=0),
        cv.Optional(CONF_TEMPERATURE_1): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_TEMPERATURE_2): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_MODE): sensor.sensor_schema(accuracy_decimals=0),
        cv.Optional(CONF_CHARGING_STATE): sensor.sensor_schema(accuracy_decimals=0),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ID])

    if voltage_config := config.get(CONF_VOLTAGE):
        sens = await sensor.new_sensor(voltage_config)
        cg.add(parent.set_telemetry_voltage_sensor(sens))

    if current_config := config.get(CONF_CURRENT):
        sens = await sensor.new_sensor(current_config)
        cg.add(parent.set_telemetry_current_sensor(sens))

    if counter_config := config.get(CONF_CHARGE_COUNTER):
        sens = await sensor.new_sensor(counter_config)
        cg.add(parent.set_charge_counter_sensor(sens))

    if t1_config := config.get(CONF_TEMPERATURE_1):
        sens = await sensor.new_sensor(t1_config)
        cg.add(parent.set_temperature1_sensor(sens))

    if t2_config := config.get(CONF_TEMPERATURE_2):
        sens = await sensor.new_sensor(t2_config)
        cg.add(parent.set_temperature2_sensor(sens))

    if mode_config := config.get(CONF_MODE):
        sens = await sensor.new_sensor(mode_config)
        cg.add(parent.set_mode_sensor(sens))

    if state_config := config.get(CONF_CHARGING_STATE):
        sens = await sensor.new_sensor(state_config)
        cg.add(parent.set_charging_state_sensor(sens))
