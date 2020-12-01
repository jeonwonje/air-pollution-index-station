// Jeon Wonje <https://github.com/jeonwonje>

#include <DHT.h>
#include <WiFi101.h>
#include <ThingSpeak.h>
#include <ArduinoLowPower.h>
#include "secrets.h"

#define LENG 31 // 0x42 + 31 bytes equal to 32 bytes
#define DHTTYPE DHT22
#define DHTPIN 2 // Pin D2 

unsigned char buf[LENG];
int PM01Value, PM2_5Value, PM10Value;

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password

float temperature, humidity;

unsigned long channelNum = SECRET_CH_ID;
const char * writeAPIKey = SECRET_WRITE_APIKEY;

WiFiClient client;

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
	if(WiFi.status() != WL_CONNECTED){
		Serial.print("Attempting to connect to SSID: ");
		Serial.println(SECRET_SSID);
		while(WiFi.status() != WL_CONNECTED){
    		WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
    		delay(5000);     
    	} 
    	Serial.println("Connected.");
    }
    Serial.begin(9600); // Serial0 is for comms between MKR1000 and computer
    Serial1.begin(9600); // Make SEN0177 use Serial1 on the MKR1000, 
	Serial1.setTimeout(1500); // set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor
	dht.begin();
	ThingSpeak.begin(client);  // Initialize ThingSpeak 
}

void loop()
{
	temperature = dht.readTemperature();
	humidity = dht.readHumidity();
	if (Serial1.find(0x42))
	{
		// start to read when detect 0x42
		Serial1.readBytes(buf, LENG);
		if (buf[0] == 0x4d)
		{
			if (checkValue(buf, LENG))
			{
				PM01Value = transmitPM01(buf); // count PM1.0 value of the air detector module
				PM2_5Value = transmitPM2_5(buf); // count PM2.5 value of the air detector module
				PM10Value = transmitPM10(buf); // count PM10 value of the air detector module
			}
		}
	}

	thingSend();
	LowPower.sleep(60 * 1000);

	/*
	Serial.print("PM1.0: ");
	Serial.print(PM01Value);
	Serial.println("  ug/m3");
	Serial.print("PM2.5: ");
	Serial.print(PM2_5Value);
	Serial.println("  ug/m3");
	Serial.print("PM1 0: ");
	Serial.print(PM10Value);
	Serial.println("  ug/m3");
	Serial.println();
	Serial.println(temperature);
	Serial.println(humidity);
	*/
}

void thingSend() 
{
	ThingSpeak.setField(1, temperature);
	ThingSpeak.setField(2, humidity);
	ThingSpeak.setField(3, PM01Value);
	ThingSpeak.setField(4, PM2_5Value);
	ThingSpeak.setField(5, PM10Value);
	int x = ThingSpeak.writeFields(channelNum, writeAPIKey);
	if(x == 200){
		Serial.println("Channel update successful.");
	}
	else{
		Serial.println("Problem updating channel. HTTP error code " + String(x));
	}
}

char checkValue(unsigned char * thebuf, char leng)
{
	char receiveflag = 0;
	int receiveSum = 0;
	for (int i = 0; i < (leng - 2); i++)
	{
		receiveSum = receiveSum + thebuf[i];
	}
	receiveSum = receiveSum + 0x42;
	if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1]))
		//check the serial data
	{
		receiveSum = 0;
		receiveflag = 1;
	}
	return receiveflag;
}

int transmitPM01(unsigned char * thebuf)
{
	int PM01Val;
	PM01Val = ((thebuf[3] << 8) + thebuf[4]); // count PM1.0 value of the air detector module
	return PM01Val;
}

// transmit PM Value to PC
int transmitPM2_5(unsigned char * thebuf)
{
	int PM2_5Val;
	PM2_5Val = ((thebuf[5] << 8) + thebuf[6]); // count PM2.5 value of the air detector module
	return PM2_5Val;
}

// transmit PM Value to PC
int transmitPM10(unsigned char * thebuf)
{
	int PM10Val;
	PM10Val = ((thebuf[7] << 8) + thebuf[8]); // count PM10 value of the air detector module
	return PM10Val;
}