#ifndef __Disco_Sensors_H__
#define __Disco_Sensors_H__

#include <DallasTemperature.h>
#include <OneWire.h>
#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>
#include <Wire.h>

#define CCS811_ADDR 0x5A
#define BME280_ADRR 0x76
#define ONE_WIRE_BUS 4

class Disco_Sensors {
public:
  /**
   *  Constructor
   */
  Disco_Sensors(int array_size);

  /**
   *  Initialises the Disco_Sensors Library
   */
  void begin();

  /**
   *  Read DS18B20
   */
  float readDS();

  /**
   *  Read Temperature of BME
   */
  float readTempBME();

  /**
   *  Read Humidity of BME
   */
  float readHumiBME();

  /**
   *  Read Algorithm Results from CSS Sensor
   */
  bool readCSS();

  /**
   *  Read CO2 from CSS Algorithm
   */
  uint16_t readCO2CSS();

  /**
   *  Read TVOC from CSS Algorithm
   */
  uint16_t readTVOCCSS();

  /**
   *  Get readings from every sensors on board and store them
   */
  void getReadings(String *data_array);

  /**
   *  Set Alarm Value for the Sensor choosen
   */
  void setAlarmSensor(String parameter, float min, float max);

  /**
   *  Check Sensors values to trigger the alarm
   */
  bool checkSensorsTreshold();

  /**
   *  Print value sensors
   */
  void printValues(String *data_array);

private:
  float tempC;
  float tempBME;
  float humiBME;
  uint16_t co2CCS;
  uint16_t tvocCCS;
  bool bmeDisconnected;
  bool readAlgorithm;

  // Status Alarm Variables
  bool alarmTemp;
  bool alarmHumi;
  bool alarmCO2;
  bool alarmTVOC;

  // Treshold Variables
  float temp_min, temp_max;
  float humi_min, humi_max;
  uint16_t co2_min, co2_max;
  uint16_t tvoc_min, tvoc_max;

  int my_array_size;
};
#endif