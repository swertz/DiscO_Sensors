# DiscO_Sensors

Library developed in order to communicate with the different sensors connected to a DiscO board:
- a board featuring the DS18B20, BME280 and CCS811 sensor
- an SCD30 sensor
- a Linduino board connected to a LTC2983 ADC reading out several analog temperature sensors (RTDs). For the firmware of the Linduino board itself, see https://cp3-git.irmp.ucl.ac.be/phase2tracker/lindui2983rtd

After retrieving the data, you can integrate theses into to general [SlowControl library](https://cp3-git.irmp.ucl.ac.be/phase2tracker/slowcontrol) which will allow you to publish data trough MQTT Network.
