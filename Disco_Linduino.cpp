#include <Disco_Linduino.h>

Disco_Linduino::Disco_Linduino(Channels &&channels, uint16_t sda, uint16_t scl):
    _channels(channels),
    _sda_pin(sda),
    _scl_pin(scl) {};

void Disco_Linduino::begin() {
    Wire.begin(_sda_pin, _scl_pin);
}

bool convertToFloat(float &temp, byte sign, byte integral, byte decimal) {
    if (sign == 2) {
        return false;
    }
    temp = static_cast<float>(integral) + static_cast<float>(decimal) / 0x100;
    if (sign == 1)
        temp = -temp;
    return true;
}

SensorValues Disco_Linduino::getReadings() {
    byte msg_length = channelCount() * 3;
    byte returned = Wire.requestFrom(DEVICE_ADDRESS, static_cast<std::size_t>(msg_length));
    Serial.println("Received " + String(returned) + " bytes");
    SensorValues values;

    if (msg_length != returned) {
        Serial.println("Error: Linduino returned " + String(returned) + " bytes while " + String(msg_length) + " were expected");
        return values;
    }
    std::vector<byte> msg;
    msg.reserve(msg_length);
    for (std::size_t i = 0; i < returned; i++) {
        if (!Wire.available()) {
            Serial.println("Error: Linduino returned only " + String(i) + " bytes while " + String(msg_length) + " were expected");
            return values;
        }
        msg.push_back(Wire.read());
        Serial.print(msg[i]);
    }
    Serial.println();

    values.reserve(channelCount());
    for (std::size_t i = 0; i < channelCount(); i++) {
        String topic(TEMP_TOPIC);
        topic.replace("#NAME#", _channels[i].first);
        float temp = LOWEST;
        if (convertToFloat(msg[3 * i], msg[3 * i + 1], msg[3 * i + 2])) {
            values.push_back({topic, String(reading)});
        } else {
            Serial.println("Error reading temperature for channel " + _channels[i].first);
        }
    }

    return values;
}

void Disco_Linduino::printValues(const SensorValues &values) {
  for (const auto &reading : values) {
    Serial.println(reading[0] + " -> " + reading[1]);
  }
}
