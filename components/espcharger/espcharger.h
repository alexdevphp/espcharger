#pragma once

#include <vector>

#include "esphome/components/button/button.h"
#include "esphome/components/number/number.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace espcharger {

class ESPChargerComponent : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;
  float get_setup_priority() const override;
  float get_loop_priority() const override;

  void set_telemetry_voltage_sensor(sensor::Sensor *sensor) { this->telemetry_voltage_sensor_ = sensor; }
  void set_telemetry_current_sensor(sensor::Sensor *sensor) { this->telemetry_current_sensor_ = sensor; }
  void set_charge_counter_sensor(sensor::Sensor *sensor) { this->charge_counter_sensor_ = sensor; }
  void set_temperature1_sensor(sensor::Sensor *sensor) { this->temperature1_sensor_ = sensor; }
  void set_temperature2_sensor(sensor::Sensor *sensor) { this->temperature2_sensor_ = sensor; }
  void set_mode_sensor(sensor::Sensor *sensor) { this->mode_sensor_ = sensor; }
  void set_charging_state_sensor(sensor::Sensor *sensor) { this->charging_state_sensor_ = sensor; }

  void set_voltage_number(number::Number *number) { this->voltage_number_ = number; }
  void set_current_number(number::Number *number) { this->current_number_ = number; }

  void set_charging_switch(switch_::Switch *sw) { this->charging_switch_ = sw; }
  void set_enable_edit_button(button::Button *button) { this->enable_edit_button_ = button; }
  void set_get_telemetry_button(button::Button *button) { this->get_telemetry_button_ = button; }

  void set_voltage(float voltage);
  void set_current(float current);
  void start_charging();
  void stop_charging();
  void enable_edit();
  void request_telemetry();

 protected:
  class ChargerSwitch;

  static uint16_t crc16_cms_(const uint8_t *data, size_t length);
  static uint16_t crc16_cms_(const std::vector<uint8_t> &data);
  static uint16_t to_le_u16_(float value, float multiplier);

  void send_frame_(uint8_t msg_type, uint16_t address, const std::vector<uint8_t> &payload);
  bool parse_frame_(std::vector<uint8_t> &frame);
  bool parse_telemetry_(const std::vector<uint8_t> &payload);

  void write_register_(uint16_t address, const std::vector<uint8_t> &payload);
  void set_switch_state_(bool state);

  std::vector<uint8_t> rx_buffer_;

  sensor::Sensor *telemetry_voltage_sensor_{nullptr};
  sensor::Sensor *telemetry_current_sensor_{nullptr};
  sensor::Sensor *charge_counter_sensor_{nullptr};
  sensor::Sensor *temperature1_sensor_{nullptr};
  sensor::Sensor *temperature2_sensor_{nullptr};
  sensor::Sensor *mode_sensor_{nullptr};
  sensor::Sensor *charging_state_sensor_{nullptr};

  number::Number *voltage_number_{nullptr};
  number::Number *current_number_{nullptr};

  switch_::Switch *charging_switch_{nullptr};
  button::Button *enable_edit_button_{nullptr};
  button::Button *get_telemetry_button_{nullptr};
};

class ESPChargerVoltageNumber : public number::Number, public Parented<ESPChargerComponent> {
 public:
  void control(float value) override;
};

class ESPChargerCurrentNumber : public number::Number, public Parented<ESPChargerComponent> {
 public:
  void control(float value) override;
};

class ESPChargerChargingSwitch : public switch_::Switch, public Parented<ESPChargerComponent> {
 public:
  void write_state(bool state) override;
};

class ESPChargerEnableEditButton : public button::Button, public Parented<ESPChargerComponent> {
 public:
  void press_action() override;
};

class ESPChargerGetTelemetryButton : public button::Button, public Parented<ESPChargerComponent> {
 public:
  void press_action() override;
};

}  // namespace espcharger
}  // namespace esphome
