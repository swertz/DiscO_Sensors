#include <Disco_Sensors.h>

float convertHumiToDP(float temp, float humi) {
  // Magnus formula, values from Sonntag (1990), 0.35°C error for -45<T<60
  static const float c = 243.12;
  static const float b = 17.62;

  const float gm = logf(humi / 100.0) + b * temp / (c + temp);

  return c * gm / (b - gm);
}

void Disco_Sensors::Disco_Sensors(uint16_t sda = SDA_PIN, uint16_t scl = SCL_PIN, uint16_t onewire = ONE_WIRE_BUS):
    _sda_pin = sda,
    _scl_pin = scl,
    _onewire_bus = onewire) {}

void Disco_Sensors::begin() {
  if (_active_ds) {
    Serial.println(F("Attempting to connect to DS18B20 sensor over 1-wire ") +
                   String(_onewire_bus));
    _oneWire = new OneWire(_onewire_bus);
    _ds = new DallasTemperature(_oneWire);
    _ds->begin();
    if (_ds->getDS18Count() < 1) {
      Serial.println(F("DS sensor did not respond, please check wiring. "
                       "Deactivating it and moving on..."));
      _active_ds = false;
    }
  }

  _active_i2c = _active_bme || _active_ccs || _active_scd;
  if (_active_i2c) {
    // Start IC Communication
    Serial.println("Initiating I2C over pins " + String(_sda_pin) + " and " +
                   String(_scl_pin));
    Wire.begin(_sda_pin, _scl_pin);
  }

  if (_active_bme) {
    _bme = new BME280();
    _bme->setI2CAddress(BME280_ADDR);
    Serial.println(F("Attempting to connect to BME280 sensor over I2C"));
    if (!_bme->beginI2C()) {
      Serial.println(F("BME280 sensor did not respond, please check address "
                       "and wiring. Deactivating it and moving on..."));
      _active_bme = false;
    }
  }

  if (_active_ccs) {
    _ccs = new CCS811(CCS811_ADDR);
    Serial.println(F("Attempting to connect to CCS811 sensor over I2C"));
    if (!_ccs->begin()) {
      Serial.print(F("CCS811 sensor did not respond, please check address and "
                     "wiring. Deactivating it and moving on..."));
      _active_ccs = false;
    }
  }

  if (_active_scd) {
    _scd = new SCD30();
    Serial.println(F("Attempting to connect to SCD30 sensor over I2C"));
    if (!_scd->begin()) {
      Serial.print(F("SCD30 sensor did not respond, please check address and "
                     "wiring. Deactivating it and moving on..."));
      _active_scd = false;
    }
  }
}

bool Disco_Sensors::readDS(float &temp) {
  if (!_active_ds)
    return false;

  // Request Temperature
  _ds->requestTemperatures();

  // Get the readings
  temp = _ds->getTempCByIndex(DS_WIRE_POS);

  return true;
}

bool Disco_Sensors::readTempBME(float &temp) {
  if (!_active_bme)
    return false;

  float tempBME = _bme->readTempC();

  if (isnan(tempBME)) {
    _bmeDisconnected = true;
    return false;
  } else {
    if (!_bmeDisconnected) {
      temp = tempBME;
    } else {
      if (_bme->beginI2C()) {
        _bmeDisconnected = false;
      }
      temp = 0.;
    }
    return true;
  }
}

bool Disco_Sensors::readHumiBME(float &humi) {
  if (!_active_bme)
    return false;

  float humiBME = _bme->readFloatHumidity();

  if (isnan(humiBME)) {
    _bmeDisconnected = true;
    return false;
  } else {
    if (!_bmeDisconnected) {
      humi = humiBME;
    } else {
      if (_bme->beginI2C()) {
        _bmeDisconnected = false;
      }
      humi = 0.00;
    }
    return true;
  }
}

bool Disco_Sensors::readCO2CCS(uint16_t &co2) {
  if (!_active_ccs)
    return false;

  auto co2CCS = _ccs->getCO2();

  if (!_ccsReadAlgorithm) {
    if (readCCS()) {
      _ccsReadAlgorithm = true;
      co2 = co2CCS;
    } else {
      co2 = 65535;
    }
  } else {
    _ccsReadAlgorithm = false;
    co2 = co2CCS;
  }
  return true;
}

bool Disco_Sensors::readTVOCCCS(uint16_t &tvoc) {
  if (!_active_ccs)
    return false;

  float tvocCCS = _ccs->getTVOC();

  if (!_ccsReadAlgorithm) {
    if (readCCS()) {
      _ccsReadAlgorithm = true;
      tvoc = tvocCCS;
    } else {
      tvoc = 65535;
    }
  } else {
    _ccsReadAlgorithm = false;
    tvoc = tvocCCS;
  }
  return true;
}

bool Disco_Sensors::readCCS() {
  if (_ccs->dataAvailable()) {
    _ccs->readAlgorithmResults();
    return true;
  } else {
    return false;
  }
}

bool Disco_Sensors::readCO2SCD(float &co2, float &temp, float &humi) {
  if (!_active_scd)
    return false;

  if (_scd->dataAvailable()) {
    co2 = _scd->getCO2();
    temp = _scd->getTemperature();
    humi = _scd->getHumidity();
    return true;
  } else {
    return false;
  }
}

bool Disco_Sensors::readTempSCD(float &temp) {
  if (!_active_scd)
    return false;

  if (_scd->dataAvailable()) {
    temp = _scd->getTemperature();
    return true;
  } else {
    return false;
  }
}

bool Disco_Sensors::readHumiSCD(float &humi) {
  if (!_active_scd)
    return false;

  if (_scd->dataAvailable()) {
    humi = _scd->getHumidity();
    return true;
  } else {
    return false;
  }
}

SensorValues Disco_Sensors::getReadings() {
  SensorValues values;

  float temp_compensate = LOWEST;
  float humi_compensate = LOWEST;

  // Read DS18B20
  float ds_temp;
  if (readDS(ds_temp)) {
    values.push_back({DS_TEMP_TOPIC, String(ds_temp)});
    temp_compensate = ds_temp;
  }

  // Read BME
  float bme_temp = LOWEST, bme_humi = LOWEST;
  if (readTempBME(bme_temp)) {
    values.push_back({BME_TEMP_TOPIC, String(bme_temp)});
    if (temp_compensate == LOWEST) {
      temp_compensate = bme_temp;
    }
  }
  if (readHumiBME(bme_humi)) {
    values.push_back({BME_HUMI_TOPIC, String(bme_humi)});
    humi_compensate = bme_humi;
  }
  if (bme_temp != LOWEST && bme_humi != LOWEST) {
    values.push_back(
        {BME_DEW_TOPIC, String(convertHumiToDP(bme_temp, bme_humi))});
  }

  // Read CCS
  uint16_t ccs_co2;
  if (readCO2CCS(ccs_co2)) {
    values.push_back({CCS_CO2_TOPIC, String(ccs_co2)});
  }
  uint16_t ccs_tvoc;
  if (readTVOCCCS(ccs_tvoc)) {
    values.push_back({CCS_TVOC_TOPIC, String(ccs_tvoc)});
  }

  // Set Environmental Data for compensation for next readings
  if (_active_ccs && temp_compensate != LOWEST && humi_compensate != LOWEST) {
    _ccs->setEnvironmentalData(humi_compensate, temp_compensate);
  }

  // Read SCD30
  float scd_co2, scd_temp, scd_humi;
  if (readSCD(scd_co2, scd_temp, scd_humi)) {
    values.push_back({SCD_CO2_TOPIC, String(scd_co2)});
    values.push_back({SCD_TEMP_TOPIC, String(scd_temp)});
    values.push_back({SCD_HUMI_TOPIC, String(scd_humi)});
    values.push_back(
        {SCD_DEW_TOPIC, String(convertHumiToDP(scd_temp, scd_humi))});
  }

  return values;
}

/*void Disco_Sensors::setAlarmSensor(String parameter, float min, float max) {
  if (parameter == "TEMP") {
    temp_min = min;
    temp_max = max;
    alarmTemp = true;
  } else if (parameter == "HUMI") {
    humi_min = min;
    humi_max = max;
    alarmHumi = true;
  } else if (parameter == "CO2") {
    co2_min = uint16_t(min);
    co2_max = uint16_t(max);
    alarmCO2 = true;
  } else if (parameter == "TVOC") {
    tvoc_min = uint16_t(min);
    tvoc_max = uint16_t(max);
    alarmTVOC = true;
  } else {
  }
}

bool Disco_Sensors::checkSensorsTreshold() {
  // Check Temp Alarm
  if (alarmTemp) {
    if (tempC > temp_max || tempC < temp_min) {
      return true;
    }

    else if (tempBME > temp_max || tempBME < temp_min) {
      return true;
    }
  }

  // Check Humi Alarm
  if (alarmHumi) {
    if (humiBME > humi_max || humiBME < humi_min) {
      return true;
    }
  }

  // Check CO2 Alarm
  if (alarmCO2) {
    if (co2CCS > co2_max || co2CCS < co2_min) {
      return true;
    }
  }

  // Check TVOC Alarm
  if (alarmTVOC) {
    if (tvocCCS > tvoc_max || tvocCCS < tvoc_min) {
      return true;
    }
  }

  return false;
}*/

void Disco_Sensors::printValues(const SensorValues &values) {
  for (const auto &reading : values) {
    Serial.println(reading[0] + " -> " + reading[1]);
  }
}
