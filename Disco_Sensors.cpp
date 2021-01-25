#include <Disco_Sensors.h>

//Objects Declaration
OneWire oneWire(ONE_WIRE_BUS);

BME280 my_bme;

CCS811 my_ccs(CCS811_ADDR);

//Pass Wire Reference
DallasTemperature sensors(&oneWire);

Disco_Sensors::Disco_Sensors(int array_size)
{
   tempC = 0.00;
   tempBME = 0.00;
   humiBME = 0.00;
   bmeDisconnected = false;
   readAlgorithm = false;
   my_array_size = array_size;
   
   //Default alarms setup
   alarmTemp = false;
   alarmHumi = false;
   alarmCO2 = false;
   alarmTVOC = false;
   
   //Declare Array of Strings to store data
   //String* data_array [array_size];
}

void Disco_Sensors::begin()
{
	//Start DS18B20 Sensors
	sensors.begin();
	
	//Start IC Communication 
	Wire.begin(13,14);

	//Start BME Sensor
	if(my_bme.beginI2C() == false)
	{
		Serial.println("The sensor did not respond. Please check wiring.");
		while(1); //Freeze
	}
	
	//Start CCS Sensor
	if (my_ccs.begin() == false)
	{
		Serial.print("CCS811 error. Please check wiring. Freezing...");
		while (1);
	}
}

float Disco_Sensors::readDS()
{
	//Request Temperature 
	sensors.requestTemperatures();
	
	//Get the readings
	tempC = sensors.getTempCByIndex(0);
	
	return tempC;
}

float Disco_Sensors::readTempBME()
{
	tempBME = my_bme.readTempC();
	
	if(isnan(tempBME))
	{
		bmeDisconnected = true;
		return NAN;
	}
	else
	{
		if(bmeDisconnected == false)
		{
			return tempBME;
		}
		else
		{
			if(my_bme.beginI2C()!=false)
			{
				bmeDisconnected = false;
			}
			
			return 0.00;
		}
	}
}

float Disco_Sensors::readHumiBME()
{
	humiBME = my_bme.readFloatHumidity();

	if(isnan(humiBME))
	{
		bmeDisconnected = true;
		return NAN;
	}
	else
	{
		if(bmeDisconnected == false)
		{
			return humiBME;
		}
		else
		{
			if(my_bme.beginI2C()!=false)
			{
				bmeDisconnected = false;
			}
			return 0.00;
		}
	}
}

uint16_t Disco_Sensors::readCO2CSS()
{
	co2CCS = my_ccs.getCO2();
	
	if(readAlgorithm == false)
	{
		if(readCSS())
		{
			readAlgorithm = true;
			return co2CCS;
		}
		else
		{
			return 65535;
		}
		
	}
	else
	{
		readAlgorithm = false;
		return co2CCS;
	}
}

uint16_t Disco_Sensors::readTVOCCSS()
{
	tvocCCS = my_ccs.getTVOC();
	
	if(readAlgorithm == false)
	{
		if(readCSS())
		{
			readAlgorithm = true;
			return tvocCCS;
		}
		else
		{
			return 65535;
		}
	}
	else
	{	
		readAlgorithm = false;	
		return tvocCCS;;		
	}
}

void Disco_Sensors::getReadings(String *data_array)
{
	//Read DS18B20
	float ds_temp_compensate = readDS();
	data_array[0] = String(ds_temp_compensate);
	
	//Read BME - Temperature
	data_array[1] = String(readTempBME());
	
	//Read BME - Humidity
	float bme_humi_compensate = readHumiBME();
	data_array[2] = String(bme_humi_compensate);
	
	//Read CCS - CO2
	data_array[3] = String(readCO2CSS());
	
	//Read CCS - TVOC
	data_array[4] = String(readTVOCCSS());
	
	//Set Environmental Data for compensation for next readings
	my_ccs.setEnvironmentalData(ds_temp_compensate,bme_humi_compensate);
	
}

bool Disco_Sensors::readCSS()
{
	if(my_ccs.dataAvailable())
	{
		my_ccs.readAlgorithmResults();
		
		return true;
	}
	else
	{
		return false;
	}
}

void Disco_Sensors::setAlarmSensor(String parameter,float min, float max)
{
	if(parameter == "TEMP")
	{
		temp_min = min;
		temp_max = max;
		alarmTemp = true;
	}
	else if (parameter == "HUMI")
	{
		humi_min = min;
		humi_max = max;
		alarmHumi = true;
	}
	else if (parameter == "CO2")
	{
		co2_min = uint16_t(min);
		co2_max = uint16_t(max);
		alarmCO2 = true;
	}
	else if (parameter == "TVOC")
	{
		tvoc_min = uint16_t(min);
		tvoc_max = uint16_t(max);
		alarmTVOC = true;
	}
	else
	{
		
	}
}

bool Disco_Sensors::checkSensorsTreshold()
{
	//Check Temp Alarm
	if(alarmTemp)
	{
		if(tempC > temp_max || tempC < temp_min)
		{
			return true;
		}

		else if(tempBME > temp_max || tempBME < temp_min)
		{
			return true;
		}
	}
	
	//Check Humi Alarm
	if(alarmHumi)
	{
		if(humiBME > humi_max || humiBME < humi_min)
		{
			return true;
		}
	}
	
	//Check CO2 Alarm
	if(alarmCO2)
	{
		if(co2CCS > co2_max || co2CCS < co2_min)
		{
			return true;
		}
	}

	
	//Check TVOC Alarm
	if(alarmTVOC)
	{
		if(tvocCCS > tvoc_max || tvocCCS < tvoc_min)
		{
			return true;
		}
	}
	
	return false;
}

void Disco_Sensors::printValues(String *data_array)
{
	for (int i=0;i<data_array->length();i++)
	{
		Serial.println(data_array[i]);
	}
}