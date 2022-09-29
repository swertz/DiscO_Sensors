#ifndef __Disco_Sensors_H__
#define __Disco_Sensors_H__

#include <DallasTemperature.h>
#include <OneWire.h>
#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>
#include <SparkFun_SCD30_Arduino_Library.h>
#include <Wire.h>
#include <vector>
#include <limits>

#define CCS811_ADDR 0x5A
#define BME280_ADDR 0x76
#define SCD30_ADDR 0x61
#define DS_WIRE_POS 0  // position along the 1-wire chain

//#define SDA_PIN 2  // for SCD30 board
#define SDA_PIN 13  // for DS/BME board
#define SCL_PIN 14
#define ONE_WIRE_BUS 4

// Topics Definitions
#define DS_TEMP_TOPIC F("/sensors/DS/temp")
#define BME_TEMP_TOPIC F("/sensors/BME/temp")
#define BME_HUMI_TOPIC F("/sensors/BME/humi")
#define BME_DEW_TOPIC F("/sensors/BME/dew")
#define CCS_CO2_TOPIC F("/sensors/CCS/co2")
#define CCS_TVOC_TOPIC F("/sensors/CCS/tvoc")
#define SCD_TEMP_TOPIC F("/sensors/SCD30/temp")
#define SCD_HUMI_TOPIC F("/sensors/SCD30/humi")
#define SCD_DEW_TOPIC F("/sensors/SCD30/dew")
#define SCD_CO2_TOPIC F("/sensors/SCD30/co2")

#define LOWEST std::numeric_limits<float>::lowest()

// holds { topic, value } for every measurement
typedef std::vector<std::vector<String>> SensorValues;

float convertHumiToDP(float temp, float humi);

class Disco_Sensors {
public:
  /**
   *  Constructor
   */
  Disco_Sensors(uint16_t sda = SDA_PIN, uint16_t scl = SCL_PIN, uint16_t onewire = ONE_WIRE_BUS);

  void activateDS(bool active=true) { _active_ds = active; }
  void activateBME(bool active=true) { _active_bme = active; }
  void activateCCS(bool active=true) { _active_ccs = active; }
  void activateSCD(bool active=true) { _active_scd = active; }

  /**
   *  Initialises the Disco_Sensors Library
   */
  void begin();

  /**
   *  Read DS18B20
   */
bool readDS(float &temp);

  /**
   *  Read Temperature of BME
   */
bool readTempBME(float &temp);

  /**
   *  Read Humidity of BME
   */
bool readHumiBME(float &humi);
  /**
   *  Read Algorithm Results from CCS Sensor
   */
  bool readCCS();

  /**
   *  Read CO2 from CCS Algorithm
   */
bool readCO2CCS(uint16_t &co2);

  /**
   *  Read TVOC from CCS Algorithm
   */
bool readTVOCCCS(uint16_t &tvoc);

  /**
   *  Read values from SCD sensor
   */
bool readSCD(float &co2, float &temp, float &humi);
  /**
   *  Get readings from every sensors on board
   */
  SensorValues getReadings();

  /**
   *  Set Alarm Value for the Sensor choosen
   */
  //void setAlarmSensor(String parameter, float min, float max);

  /**
   *  Check Sensors values to trigger the alarm
   */
  //bool checkSensorsTreshold();

  /**
   *  Print value sensors
   */
  void printValues(const SensorValues& values);

private:
  bool _active_ds = false;
  bool _active_bme = false;
  bool _active_ccs = false;
  bool _active_scd = false;
  bool _active_i2c = false;

  uint16_t _sda_pin;
  uint16_t _scl_pin;
  uint16_t _onewire_bus;

  OneWire* _oneWire = nullptr;
  DallasTemperature* _ds = nullptr;
  BME280* _bme = nullptr;
  CCS811* _ccs = nullptr;
  SCD30* _scd = nullptr;

  bool _bmeDisconnected = false;
  bool _ccsReadAlgorithm = false;

  //// Status Alarm Variables
  //bool alarmTemp;
  //bool alarmHumi;
  //bool alarmCO2;
  //bool alarmTVOC;

  //// Treshold Variables
  //float temp_min, temp_max;
  //float humi_min, humi_max;
  //uint16_t co2_min, co2_max;
  //uint16_t tvoc_min, tvoc_max;
};
#endif
