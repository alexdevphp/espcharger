#include "espcharger.h"

#include <cmath>

#include "esphome/core/log.h"

namespace esphome {
namespace espcharger {

static const char *const TAG = "espcharger";
static const uint8_t FRAME_HEADER_1 = 0x11;
static const uint8_t FRAME_HEADER_2 = 0x55;

void ESPChargerComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ESPCharger UART component...");
  this->enable_edit();
}

void ESPChargerComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ESPCharger:");
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Telemetry Voltage", this->telemetry_voltage_sensor_);
  LOG_SENSOR("  ", "Telemetry Current", this->telemetry_current_sensor_);
  LOG_SENSOR("  ", "Charge Counter", this->charge_counter_sensor_);
  LOG_SENSOR("  ", "Temperature 1", this->temperature1_sensor_);
  LOG_SENSOR("  ", "Temperature 2", this->temperature2_sensor_);
  LOG_SENSOR("  ", "Mode", this->mode_sensor_);
  LOG_SENSOR("  ", "Charging State", this->charging_state_sensor_);
}

void ESPChargerComponent::update() { this->request_telemetry(); }

void ESPChargerComponent::loop() {
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    this->rx_buffer_.push_back(byte);
  }

  while (this->rx_buffer_.size() >= 8) {
    if (this->rx_buffer_[0] != FRAME_HEADER_1 || this->rx_buffer_[1] != FRAME_HEADER_2) {
      this->rx_buffer_.erase(this->rx_buffer_.begin());
      continue;
    }

    const uint8_t payload_len = this->rx_buffer_[5];
    const size_t frame_len = 2 + 1 + 2 + 1 + payload_len + 2;
    if (this->rx_buffer_.size() < frame_len)
      return;

    std::vector<uint8_t> frame(this->rx_buffer_.begin(), this->rx_buffer_.begin() + frame_len);
    this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + frame_len);
    this->parse_frame_(frame);
  }
}

void ESPChargerComponent::set_voltage(float voltage) {
  const float v = std::max(10.0f, std::min(93.0f, voltage));

  std::vector<uint8_t> p1;
  p1.reserve(16);
  p1.push_back(0x01);
  p1.push_back(0x04);
  const uint16_t x48_5 = to_le_u16_(v, 48.5f);
  p1.push_back(static_cast<uint8_t>(x48_5 & 0xFF));
  p1.push_back(static_cast<uint8_t>((x48_5 >> 8) & 0xFF));
  p1.push_back(0x64);
  p1.push_back(0x00);
  const uint16_t v100 = to_le_u16_(v, 100.0f);
  for (int i = 0; i < 4; i++) {
    p1.push_back(static_cast<uint8_t>(v100 & 0xFF));
    p1.push_back(static_cast<uint8_t>((v100 >> 8) & 0xFF));
  }
  this->write_register_(0x0102, p1);

  std::vector<uint8_t> p2;
  p2.reserve(8);
  const uint16_t x81 = to_le_u16_(v, 81.0f);
  p2.push_back(static_cast<uint8_t>(x81 & 0xFF));
  p2.push_back(static_cast<uint8_t>((x81 >> 8) & 0xFF));
  const uint16_t x100_m100 = static_cast<uint16_t>(std::roundf(v * 100.0f - 100.0f));
  p2.push_back(static_cast<uint8_t>(x100_m100 & 0xFF));
  p2.push_back(static_cast<uint8_t>((x100_m100 >> 8) & 0xFF));
  p2.push_back(0xFF);
  p2.push_back(0xFF);
  p2.push_back(0xFF);
  p2.push_back(0xFF);
  this->write_register_(0x011A, p2);

  std::vector<uint8_t> p3;
  p3.reserve(8);
  const uint16_t x53_3 = to_le_u16_(v, 53.3f);
  p3.push_back(static_cast<uint8_t>(x53_3 & 0xFF));
  p3.push_back(static_cast<uint8_t>((x53_3 >> 8) & 0xFF));
  const uint16_t x80 = to_le_u16_(v, 80.0f);
  p3.push_back(static_cast<uint8_t>(x80 & 0xFF));
  p3.push_back(static_cast<uint8_t>((x80 >> 8) & 0xFF));
  p3.push_back(0xB4);
  p3.push_back(0x00);
  p3.push_back(0x2C);
  p3.push_back(0x01);
  this->write_register_(0x019E, p3);

  std::vector<uint8_t> p4;
  p4.reserve(7);
  p4.push_back(0x02);
  const uint16_t x79_3 = to_le_u16_(v, 79.3f);
  p4.push_back(static_cast<uint8_t>(x79_3 & 0xFF));
  p4.push_back(static_cast<uint8_t>((x79_3 >> 8) & 0xFF));
  const uint16_t x86_1 = to_le_u16_(v, 86.1f);
  p4.push_back(static_cast<uint8_t>(x86_1 & 0xFF));
  p4.push_back(static_cast<uint8_t>((x86_1 >> 8) & 0xFF));
  const uint16_t x98_2 = to_le_u16_(v, 98.2f);
  p4.push_back(static_cast<uint8_t>(x98_2 & 0xFF));
  p4.push_back(static_cast<uint8_t>((x98_2 >> 8) & 0xFF));
  this->write_register_(0x01B9, p4);

  if (this->voltage_number_ != nullptr)
    this->voltage_number_->publish_state(v);
}

void ESPChargerComponent::set_current(float current) {
  const float c = std::max(1.0f, std::min(20.0f, current));

  std::vector<uint8_t> p1;
  p1.reserve(8);
  const uint16_t x33 = to_le_u16_(c, 33.0f);
  p1.push_back(static_cast<uint8_t>(x33 & 0xFF));
  p1.push_back(static_cast<uint8_t>((x33 >> 8) & 0xFF));
  const uint16_t c100 = to_le_u16_(c, 100.0f);
  p1.push_back(static_cast<uint8_t>(c100 & 0xFF));
  p1.push_back(static_cast<uint8_t>((c100 >> 8) & 0xFF));
  const uint16_t x80 = to_le_u16_(c, 80.0f);
  p1.push_back(static_cast<uint8_t>(x80 & 0xFF));
  p1.push_back(static_cast<uint8_t>((x80 >> 8) & 0xFF));
  p1.push_back(static_cast<uint8_t>(c100 & 0xFF));
  p1.push_back(static_cast<uint8_t>((c100 >> 8) & 0xFF));
  this->write_register_(0x012A, p1);

  std::vector<uint8_t> p2{0x00, 0x00, 0x00, 0x00, static_cast<uint8_t>((c > 14.0f) ? 200 : 80), 0x00, 0x00, 0x00};
  this->write_register_(0x013A, p2);

  if (this->current_number_ != nullptr)
    this->current_number_->publish_state(c);
}

void ESPChargerComponent::start_charging() {
  this->write_register_(0x0502, {0xA8});
  this->set_switch_state_(true);
}

void ESPChargerComponent::stop_charging() {
  this->write_register_(0x0502, {0xA1});
  this->set_switch_state_(false);
}

void ESPChargerComponent::enable_edit() { this->send_frame_(0x4C, 0x0040, {0xA4, 0x5B}); }

void ESPChargerComponent::request_telemetry() { this->send_frame_(0x52, 0x0A01, {0x18}); }

void ESPChargerComponent::write_register_(uint16_t address, const std::vector<uint8_t> &payload) {
  this->send_frame_(0x57, address, payload);
}

void ESPChargerComponent::send_frame_(uint8_t msg_type, uint16_t address, const std::vector<uint8_t> &payload) {
  std::vector<uint8_t> frame;
  frame.reserve(2 + 1 + 2 + 1 + payload.size() + 2);
  frame.push_back(FRAME_HEADER_1);
  frame.push_back(FRAME_HEADER_2);
  frame.push_back(msg_type);
  frame.push_back(static_cast<uint8_t>((address >> 8) & 0xFF));
  frame.push_back(static_cast<uint8_t>(address & 0xFF));
  frame.push_back(static_cast<uint8_t>(payload.size()));
  frame.insert(frame.end(), payload.begin(), payload.end());

  const uint16_t crc = crc16_cms_(frame);
  frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  frame.push_back(static_cast<uint8_t>(crc & 0xFF));

  this->write_array(frame);
  this->flush();
}

bool ESPChargerComponent::parse_frame_(std::vector<uint8_t> &frame) {
  const uint8_t msg_type = frame[2];
  const uint8_t payload_len = frame[5];
  const uint16_t expected_crc = (static_cast<uint16_t>(frame[frame.size() - 2]) << 8) | frame[frame.size() - 1];
  const uint16_t actual_crc = crc16_cms_(frame.data(), frame.size() - 2);

  if (expected_crc != actual_crc) {
    ESP_LOGW(TAG, "CRC mismatch: exp=0x%04X got=0x%04X", expected_crc, actual_crc);
    return false;
  }

  std::vector<uint8_t> payload(frame.begin() + 6, frame.begin() + 6 + payload_len);

  if (msg_type == 0x53) {
    return this->parse_telemetry_(payload);
  }

  return true;
}

bool ESPChargerComponent::parse_telemetry_(const std::vector<uint8_t> &payload) {
  if (payload.size() < 24) {
    ESP_LOGW(TAG, "Telemetry payload too short: %u", static_cast<unsigned>(payload.size()));
    return false;
  }

  const auto read_u16 = [&payload](size_t index) -> uint16_t {
    return static_cast<uint16_t>(payload[index]) | (static_cast<uint16_t>(payload[index + 1]) << 8);
  };

  const uint16_t voltage_raw = read_u16(4);
  const uint16_t current_raw = read_u16(6);
  const uint16_t counter_raw = read_u16(14);
  const uint16_t temp1_raw = read_u16(16);
  const uint16_t temp2_raw = read_u16(18);
  const uint8_t mode = payload[21];
  const uint8_t charging_state = payload[22];

  if (this->telemetry_voltage_sensor_ != nullptr)
    this->telemetry_voltage_sensor_->publish_state(voltage_raw / 100.0f);
  if (this->telemetry_current_sensor_ != nullptr)
    this->telemetry_current_sensor_->publish_state(current_raw / 100.0f);
  if (this->charge_counter_sensor_ != nullptr)
    this->charge_counter_sensor_->publish_state(counter_raw);
  if (this->temperature1_sensor_ != nullptr)
    this->temperature1_sensor_->publish_state(temp1_raw);
  if (this->temperature2_sensor_ != nullptr)
    this->temperature2_sensor_->publish_state(temp2_raw);
  if (this->mode_sensor_ != nullptr)
    this->mode_sensor_->publish_state(mode);
  if (this->charging_state_sensor_ != nullptr)
    this->charging_state_sensor_->publish_state(charging_state);

  this->set_switch_state_(charging_state == 1);

  return true;
}

void ESPChargerComponent::set_switch_state_(bool state) {
  if (this->charging_switch_ != nullptr)
    this->charging_switch_->publish_state(state);
}

uint16_t ESPChargerComponent::to_le_u16_(float value, float multiplier) {
  return static_cast<uint16_t>(std::roundf(value * multiplier));
}

uint16_t ESPChargerComponent::crc16_cms_(const uint8_t *data, size_t length) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; i++) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (uint8_t bit = 0; bit < 8; bit++) {
      crc = (crc & 0x8000U) ? static_cast<uint16_t>((crc << 1U) ^ 0x8005U) : static_cast<uint16_t>(crc << 1U);
    }
  }
  return crc;
}

uint16_t ESPChargerComponent::crc16_cms_(const std::vector<uint8_t> &data) { return crc16_cms_(data.data(), data.size()); }

void ESPChargerVoltageNumber::control(float value) { this->parent_->set_voltage(value); }

void ESPChargerCurrentNumber::control(float value) { this->parent_->set_current(value); }

void ESPChargerChargingSwitch::write_state(bool state) {
  if (state) {
    this->parent_->start_charging();
  } else {
    this->parent_->stop_charging();
  }
}

void ESPChargerEnableEditButton::press_action() { this->parent_->enable_edit(); }

void ESPChargerGetTelemetryButton::press_action() { this->parent_->request_telemetry(); }

}  // namespace espcharger
}  // namespace esphome
