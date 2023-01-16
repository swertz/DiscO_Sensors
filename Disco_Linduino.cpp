#include <Disco_Linduino.h>

Disco_Linduino::Disco_Linduino(Channels &&channels, uint16_t sda, uint16_t scl):
    _channels(channels),
    _sda_pin(sda),
    _scl_pin(scl) {};

void Disco_Linduino::begin() {
    Wire.begin(_sda_pin, _scl_pin);
}

SensorValues getReadings() {
    std::vector<byte> msg;
    std::size_t msg_length = sensors.channelCount() * 7;
    msg.reserve(msg_length);
    byte returned = Wire.requestFrom(DEVICE_ADDRESS, msg_length);
    returned = min(returned, msg_length);

    bool success = true;
    for (std::size_t i = 0; i < returned; i++) {
        if (!Wire.available()) {
            Serial.println("Error: Linduino returned only " + String(i) + " bytes while " + String(msg_length) + " were expected");
            success = false;
            break;
        }
        msg[i] = Wire.read();
    }

    SensorValues values;
    
    if (success) {
        values.reserve(channelCount());
        for (std::size_t i = 0; i < channelCount(); i++) {
            String topic(TEMP_TOPIC);
            topic.replace("#NAME#", _channels[i].first);
            String reading = (char*)(&msg[channels[i].second * 7]);
            values.emplace_back(topic, reading);
        }
    }

    return values;
}

void Disco_Linduino::printValues(const SensorValues &values) {
  for (const auto &reading : values) {
    Serial.println(reading[0] + " -> " + reading[1]);
  }
}
