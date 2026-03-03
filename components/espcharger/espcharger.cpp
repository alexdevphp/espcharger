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
  LOG_SENSOR("  ", "Output Voltage", this->output_voltage_sensor);
  LOG_SENSOR("  ", "Output Current", this->output_current_sensor);
  LOG_SENSOR("  ", "Charge Counter", this->charge_counter_sensor);
  LOG_SENSOR("  ", "Temperature 1", this->temperature1_sensor);
  LOG_SENSOR("  ", "Temperature 2", this->temperature2_sensor);
  LOG_SENSOR("  ", "Mode", this->mode_sensor);
  LOG_SENSOR("  ", "Charging State", this->charging_state_sensor);
}

void ESPChargerComponent::update() { }

void ESPChargerComponent::loop() { 

  if(millis() - config_req_time > 2000) {
    config_req_time = millis();
    this->request_status();
  }

  if(this->read_data()) {
    if (this->frame.msg_type == 0x53) {
      this->parse_telemetry();
    }    
  } 
}

bool ESPChargerComponent::read_data() {
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    this->rx_buffer.push_back(byte);
  }

  while (this->rx_buffer.size() >= 8) {
    if (this->rx_buffer[0] != FRAME_HEADER_1 || this->rx_buffer[1] != FRAME_HEADER_2) {
      this->rx_buffer.erase(this->rx_buffer.begin());
      continue;
    }

    const uint8_t payload_len = this->rx_buffer[5];
    const size_t frame_len = payload_len + 8;
    if (this->rx_buffer.size() < frame_len) {
      return false;
    }

    std::vector<uint8_t> fdata(this->rx_buffer.begin(), this->rx_buffer.begin() + frame_len);

    std::string hex;
    for (int i = 0; i < fdata.size(); i++) {
          char buf[4];
          snprintf(buf, sizeof(buf), "%02X ", fdata[i]);
          hex += buf;
    }
    ESP_LOGD(TAG, "<<HEX: %s", hex.c_str());

    this->rx_buffer.erase(this->rx_buffer.begin(), this->rx_buffer.begin() + frame_len);
    
    this->frame.msg_type = fdata[2];
    this->frame.payload_len = fdata[5];
    this->frame.address = (static_cast<uint16_t>(fdata[3]) << 8) | fdata[4];

    const uint16_t expected_crc = (static_cast<uint16_t>(fdata[fdata.size() - 2]) << 8) | fdata[fdata.size() - 1];
    const uint16_t actual_crc = crc16_cms(fdata.data(), fdata.size() - 2);

    if (expected_crc != actual_crc) {
      ESP_LOGW(TAG, "CRC mismatch: exp=0x%04X got=0x%04X", expected_crc, actual_crc);
      return false;
    }
    this->frame.payload.assign(fdata.begin() + 6, fdata.begin() + 6 + payload_len);
    return true;
  }
  return false;
} 

void ESPChargerComponent::set_voltage(float voltage) {
  //const float v = std::max(10.0f, std::min(this->max_voltage, voltage));
  const float v = voltage;

  const uint16_t x48_5 = std::roundf(v * 48.5f);
  const uint16_t v100 = std::roundf(v * 100.0f);
  const uint8_t msg_type = 0x57;

  std::vector<uint8_t> v1;
  v1.reserve(16);
  v1.insert(v1.end(), { 
    0x01, 0x04, 
    static_cast<uint8_t>(x48_5 & 0xFF), 
    static_cast<uint8_t>((x48_5 >> 8) & 0xFF), 
    0x64, 0x00, 0x32, 0x00 
  });

  for (int i = 0; i < 4; i++) {
    v1.push_back(static_cast<uint8_t>(v100 & 0xFF));
    v1.push_back(static_cast<uint8_t>((v100 >> 8) & 0xFF));
  }
  this->send_data(0x0102, msg_type, v1);
  
  ///----------
  const uint16_t x81 = std::roundf(v * 81.0f);
  const uint16_t x100 = std::roundf(v * 100.0f - 100.0f);
  
  std::vector<uint8_t> v2;
  v2.reserve(8);
  v2.insert(v2.end(), { 
    static_cast<uint8_t>(x81 & 0xFF), 
    static_cast<uint8_t>((x81 >> 8) & 0xFF),
    static_cast<uint8_t>(x100 & 0xFF), 
    static_cast<uint8_t>((x100 >> 8) & 0xFF),
    0xff, 0xff, 0xff, 0xff
  });
  
  this->send_data(0x011A, msg_type, v2);

  //---------
  const uint16_t x53_3 = std::roundf(v * 53.3f);
  const uint16_t x80 = std::roundf(v * 80.0f);

  std::vector<uint8_t> v3;
  v3.reserve(8);
  v2.insert(v2.end(), { 
    static_cast<uint8_t>(x53_3 & 0xFF), 
    static_cast<uint8_t>((x53_3 >> 8) & 0xFF),
    static_cast<uint8_t>(x80 & 0xFF), 
    static_cast<uint8_t>((x80 >> 8) & 0xFF),
    0xb4, 0x00, 0x2c, 0x01
  });

  this->send_data(0x019E, msg_type, v3);

  //----------
  const uint16_t x79_3 = std::roundf(v * 79.3f);
  const uint16_t x86_1 = std::roundf(v * 86.1f);
  const uint16_t x98_2 = std::roundf(v * 98.2f);

  std::vector<uint8_t> v4;
  v4.reserve(7);
  v2.insert(v2.end(), { 0x02,
    static_cast<uint8_t>(x79_3 & 0xFF), 
    static_cast<uint8_t>((x79_3 >> 8) & 0xFF),
    static_cast<uint8_t>(x86_1 & 0xFF), 
    static_cast<uint8_t>((x86_1 >> 8) & 0xFF),
    static_cast<uint8_t>(x98_2 & 0xFF), 
    static_cast<uint8_t>((x98_2 >> 8) & 0xFF),
  });
  this->send_data(0x01B9, msg_type, v4);
}

void ESPChargerComponent::set_current(float current) {
  //const float c = std::max(1.0f, std::min(this->max_current, current));
  const float c = current;
  const uint8_t msg_type = 0x57;

  const uint16_t x33 = std::roundf(c * 33.0f);
  const uint16_t c100 = std::roundf(c * 100.0f);
  const uint16_t x80 = std::roundf(c * 80.0f);

  std::vector<uint8_t> p1;
  p1.reserve(8);
  p1.insert(p1.end(), {
    static_cast<uint8_t>(x33 & 0xFF), 
    static_cast<uint8_t>((x33 >> 8) & 0xFF),
    static_cast<uint8_t>(c100 & 0xFF), 
    static_cast<uint8_t>((c100 >> 8) & 0xFF),
    static_cast<uint8_t>(x80 & 0xFF), 
    static_cast<uint8_t>((x80 >> 8) & 0xFF),
    static_cast<uint8_t>(c100 & 0xFF), 
    static_cast<uint8_t>((c100 >> 8) & 0xFF)
  });
  this->send_data(0x012A, msg_type, p1);


  const uint16_t x200 = static_cast<uint16_t>((c > 14.0f) ? 200 : 80);

  std::vector<uint8_t> p2 {
    0x00, 0x00, 0x00, 0x00,
    static_cast<uint8_t>(x200 & 0xFF), 
    static_cast<uint8_t>((x200 >> 8) & 0xFF),
    0x00, 0x00, 0x00
  };
  this->send_data(0x013A, msg_type, p2);
}

void ESPChargerComponent::start_charging() {
  const bool ok = this->send_data(0x0502, 0x57, {0xA8});
  if(ok) this->set_switch_state(true);
}

void ESPChargerComponent::stop_charging() {
  const bool ok = this->send_data(0x0502, 0x57, {0xA1});
  if(ok) this->set_switch_state(false);
}

void ESPChargerComponent::enable_edit() { 
  this->send_data(0x0040, 0x4C, {0xA4, 0x5B});
}

void ESPChargerComponent::request_status() { 
  this->send_frame(0x0102, 0x52, {0x10});
  if(wait_for_resp(0x0102, 200)) {
    const uint16_t voltage = this->read_u16(14);
    this->voltage_number->publish_state(voltage / 100.0f);
  }

  std::vector<uint8_t> p2 = {0x10};
  this->send_frame(0x012a, 0x52, {0x10});
  if(wait_for_resp(0x012a, 200)) {
    const uint16_t current = this->read_u16(6);
    this->current_number->publish_state(current / 100.0f);
  }

  this->send_frame(0x0A00, 0x52, {0x18});
  if(wait_for_resp(0x0A00, 200)) {
    parse_telemetry();
  }

}

bool ESPChargerComponent::wait_for_resp(uint16_t address, uint32_t timeout_ms) {
  const uint32_t start = millis();
  while (millis() - start < timeout_ms) {
    bool ok = this->read_data();
    if(ok && this->frame.address == address) return true;
  }
  ESP_LOGW(TAG, "Response timeout");
  return false;
}

bool ESPChargerComponent::send_data(uint16_t address, uint8_t msg_type, const std::vector<uint8_t> &payload) {
  uint8_t payload_size = payload.size();

  for (uint8_t attempt = 1; attempt <= 3; attempt++) {
    this->send_frame(address, msg_type, payload);

    if (this->wait_for_ack(address, payload_size, 200)) {
      ESP_LOGD(TAG, "SENT OK");
      return true;
    }

    ESP_LOGW(TAG, "ACK timeout/mismatch for addr=0x%04X, attempt %u", address, attempt);
  }

  return false;
}

bool ESPChargerComponent::wait_for_ack(uint16_t address, uint8_t payload_size, uint32_t timeout_ms) {
  std::vector<uint8_t> ack = {payload_size, 0x4f, 0x4b};

  const uint32_t start = millis();
  while (millis() - start < timeout_ms) {
    bool ok = this->read_data();

    if (ok && this->frame.msg_type == 0x42 && this->frame.address == address) {
      if (this->frame.payload == ack) {
        return true;
      }

      ESP_LOGW(TAG, "ACK payload mismatch addr=0x%04X expected_len=%u got_len=%u", address,
               static_cast<unsigned>(ack.size()),
               static_cast<unsigned>(this->frame.payload.size()));
      return false;
    }
    delay(1);
  }
  return false;
}


void ESPChargerComponent::send_frame(uint16_t address, uint8_t msg_type, const std::vector<uint8_t> &payload) {
  std::vector<uint8_t> frame;
  frame.reserve(payload.size() + 8);
  frame.push_back(FRAME_HEADER_1);
  frame.push_back(FRAME_HEADER_2);
  frame.push_back(msg_type);
  frame.push_back(static_cast<uint8_t>((address >> 8) & 0xFF));
  frame.push_back(static_cast<uint8_t>(address & 0xFF));
  frame.push_back(static_cast<uint8_t>(payload.size()));
  frame.insert(frame.end(), payload.begin(), payload.end());

  const uint16_t crc = crc16_cms(frame);
  frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  frame.push_back(static_cast<uint8_t>(crc & 0xFF));

  std::string hex;
  for (int i = 0; i < frame.size(); i++) {
        char buf[4];
        snprintf(buf, sizeof(buf), "%02X ", frame[i]);
        hex += buf;
  }
  ESP_LOGD(TAG, ">>HEX: %s", hex.c_str());

  this->write_array(frame);
  this->flush();
}

  
uint16_t ESPChargerComponent::read_u16(size_t index) {
  return (this->frame.payload[index]) | ((this->frame.payload[index + 1]) << 8);
};

bool ESPChargerComponent::parse_telemetry() {
  if (this->frame.payload.size() < 24) {
    ESP_LOGW(TAG, "Telemetry payload too short: %u", static_cast<unsigned>(this->frame.payload.size()));
    return false;
  }

  const uint16_t voltage_raw = this->read_u16(4);
  const uint16_t current_raw = this->read_u16(6);
  const uint16_t counter_raw = this->read_u16(14);
  const uint16_t temp1_raw = this->read_u16(16);
  const uint16_t temp2_raw = this->read_u16(18);
  const uint8_t mode = this->frame.payload[21];
  const uint8_t charging_state = this->frame.payload[22];

  if (this->output_voltage_sensor != nullptr)
    this->output_voltage_sensor->publish_state(voltage_raw / 100.0f);
  if (this->output_current_sensor != nullptr)
    this->output_current_sensor->publish_state(current_raw / 100.0f);
  if (this->charge_counter_sensor != nullptr)
    this->charge_counter_sensor->publish_state(counter_raw);
  if (this->temperature1_sensor != nullptr)
    this->temperature1_sensor->publish_state(temp1_raw);
  if (this->temperature2_sensor != nullptr)
    this->temperature2_sensor->publish_state(temp2_raw);
  if (this->mode_sensor != nullptr)
    this->mode_sensor->publish_state(mode);
  if (this->charging_state_sensor != nullptr)
    this->charging_state_sensor->publish_state(charging_state);

  this->set_switch_state(charging_state == 1);

  return true;
}

void ESPChargerComponent::set_switch_state(bool state) {
  if (this->charging_switch != nullptr)
    this->charging_switch->publish_state(state);
}

uint16_t ESPChargerComponent::to_le_u16(float value, float multiplier) {
  return static_cast<uint16_t>(std::roundf(value * multiplier));
}

uint16_t ESPChargerComponent::crc16_cms(const uint8_t *data, size_t length) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; i++) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (uint8_t bit = 0; bit < 8; bit++) {
      crc = (crc & 0x8000U) ? static_cast<uint16_t>((crc << 1U) ^ 0x8005U) : static_cast<uint16_t>(crc << 1U);
    }
  }
  return crc;
}

uint16_t ESPChargerComponent::crc16_cms(const std::vector<uint8_t> &data) { return crc16_cms(data.data(), data.size()); }

void ESPChargerVoltageNumber::control(float value) { this->parent_->set_voltage(value); }

void ESPChargerCurrentNumber::control(float value) { this->parent_->set_current(value); }

void ESPChargerChargingSwitch::write_state(bool state) {
  if (state) {
    this->parent_->start_charging();
  } else {
    this->parent_->stop_charging();
  }
}

}  // namespace espcharger
}  // namespace esphome
