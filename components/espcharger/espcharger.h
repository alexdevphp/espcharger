#pragma once

#include <vector>

#include "esphome/components/number/number.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace espcharger {

class ESPChargerComponent : public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;
  void update() override;

  void set_output_voltage_sensor(sensor::Sensor *sensor) { this->output_voltage_sensor = sensor; }
  void set_output_current_sensor(sensor::Sensor *sensor) { this->output_current_sensor = sensor; }
  void set_charge_counter_sensor(sensor::Sensor *sensor) { this->charge_counter_sensor = sensor; }
  void set_temperature1_sensor(sensor::Sensor *sensor) { this->temperature1_sensor = sensor; }
  void set_temperature2_sensor(sensor::Sensor *sensor) { this->temperature2_sensor = sensor; }
  void set_mode_sensor(sensor::Sensor *sensor) { this->mode_sensor = sensor; }
  void set_charging_state_sensor(sensor::Sensor *sensor) { this->charging_state_sensor = sensor; }
  void set_voltage_number(number::Number *number) { this->voltage_number = number; }
  void set_current_number(number::Number *number) { this->current_number = number; }
  void set_charging_switch(switch_::Switch *sw) { this->charging_switch = sw; }


  void set_voltage(float voltage);
  void set_current(float current);
  void start_charging();
  void stop_charging();
  void enable_edit();
  void request_status();

 protected:
  class ChargerSwitch;

  struct RxFrame {
    std::vector<uint8_t> payload;
    uint8_t msg_type;
    uint8_t payload_len;
    uint16_t address{0};
  };

  static uint16_t crc16_cms(const uint8_t *data, size_t length);
  static uint16_t crc16_cms(const std::vector<uint8_t> &data);
  static uint16_t to_le_u16(float value, float multiplier);
  RxFrame frame;
  
  void send_frame(uint16_t address, uint8_t msg_type, const std::vector<uint8_t> &payload);
  bool wait_for_ack(uint16_t address, uint8_t payload_size, uint32_t timeout_ms);
  bool parse_telemetry();
  bool read_data();

  bool send_data(uint16_t address, uint8_t msg_type, const std::vector<uint8_t> &payload);
  bool wait_for_resp(uint16_t address, uint32_t timeout_ms);
  void set_switch_state(bool state);
  uint16_t read_u16(size_t index);

  uint32_t config_req_time = millis();

  std::vector<uint8_t> rx_buffer;

  float max_voltage;
  float max_current;

  sensor::Sensor *output_voltage_sensor{nullptr};
  sensor::Sensor *output_current_sensor{nullptr};
  sensor::Sensor *charge_counter_sensor{nullptr};
  sensor::Sensor *temperature1_sensor{nullptr};
  sensor::Sensor *temperature2_sensor{nullptr};
  sensor::Sensor *mode_sensor{nullptr};
  sensor::Sensor *charging_state_sensor{nullptr};

  number::Number *voltage_number{nullptr};
  number::Number *current_number{nullptr};

  switch_::Switch *charging_switch{nullptr};
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

}  // namespace espcharger
}  // namespace esphome
