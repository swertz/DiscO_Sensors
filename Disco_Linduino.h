#ifndef __Disco_Linduino_H__
#define __Disco_Linduino_H__

#include <Wire.h>

#include <vector>
#include <ArduinoJson.h>

#define SDA_PIN 2  // why!!??
#define SCL_PIN 14

#define DEVICE_ADDRESS 0x08 // I2C slave ID
                            //
#define TEMP_TOPIC F("/sensors/RTD/CHAN#NAME#/temp")

// holds { topic, value } for every measurement
typedef std::vector<std::vector<String>> SensorValues;

// holds { channel name, channel number on DC2210 board }
typedef std::vector<std::pair<String, uint8_t>> Channels;

class Disco_Linduino {
public:
  /**
   *  Constructor
   */
  Disco_Linduino(Channels &&channels, uint16_t sda = SDA_PIN, uint16_t scl = SCL_PIN);

  /**
   *  Initialises the Disco_Linduino Library
   */
  void begin();

  /**
   *  Get readings from every sensors on board
   */
  SensorValues getReadings();

  /**
   *  Print value sensors
   */
  void printValues(const SensorValues &values);

  uint8_t channelCount() const { return _channels.size(); }

private:
  Channels _channels;
  uint16_t _sda_pin;
  uint16_t _scl_pin;
};
#endif
