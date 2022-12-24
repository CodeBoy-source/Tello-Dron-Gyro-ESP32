#include <ESPTelloCLI.h>
#include <GY521.h>

ESPTelloCLI TelloCLI;
GY521 sensor(0x68);
uint32_t counter = 0;

bool Connected;
bool Command;
bool commandSent;
bool takeoff;
unsigned int delvar;
char flipside;

void WiFiEvent(WiFiEvent_t event)
{
	switch (event)
	{
#ifdef ESP8266
	case WIFI_EVENT_STAMODE_GOT_IP:
#else
	case ARDUINO_EVENT_WIFI_STA_GOT_IP:
#endif
		Serial.print("WiFi connected! IP address: ");
		Serial.println(WiFi.localIP());
		TelloCLI.begin();
		// Turn off telemetry if not needed.
		TelloCLI.setTelemetry(false);
		Connected = true;
    digitalWrite(18,HIGH);
    digitalWrite(17,HIGH);
    digitalWrite(16,LOW);
		break;
#ifdef ESP8266
	case WIFI_EVENT_STAMODE_DISCONNECTED:
#else
	case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
#endif
		Serial.println("WiFi lost connection");
		TelloCLI.end();
		Connected = false;
		break;
	default:
		break;
	}
}

void setup()
{
  pinMode(18,OUTPUT);
  pinMode(17,OUTPUT);
  pinMode(16,OUTPUT);

  digitalWrite(18,HIGH);
  digitalWrite(17,LOW);
  digitalWrite(16,LOW);

  // subir
  pinMode(4, INPUT_PULLUP);
  // Bajar
  pinMode(15, INPUT_PULLUP);
  // Despegar / Aterrizar
  pinMode(25, INPUT_PULLUP);
  // Flip
  pinMode(33, INPUT_PULLUP);


	Serial.begin(115200);
	Serial.setTimeout(0);

	delvar = 50;
	Connected = false;
	Command = false;
  commandSent=false;
	takeoff = false;
	flipside = 'f'; //Front


  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiEvent);

  Wire.begin();
  Wire.setTimeOut(3000);

  delay(100);
  while (sensor.wakeup() == false) {
    Serial.print(millis());
    Serial.println("\tCould not connect to GY521");
    delay(1000);
  }
  sensor.setAccelSensitivity(2);  // 8g
  sensor.setGyroSensitivity(1);   // 500 degrees/s

  sensor.setThrottle();
  Serial.println("start...");

  // set calibration values from calibration sketch.
  sensor.axe = 0.204;
  sensor.aye = 0.011;
  sensor.aze = -0.997;
  sensor.gxe = 4.010;
  sensor.gye = 0.277;
  sensor.gze = -1.428;
}

void loop()
{
  sensor.read();

	if (!Connected)
	{
    String command = "TELLO-5ACC7D";
    Serial.println(command);
    char SSID[65];
    strcpy(SSID, command.c_str());
    WiFi.begin(SSID);
    delay(1000);
  }else if(Connected && !Command){
    String command = "command";
    TelloCLI.write(command.c_str(), command.length());
    delay(100);
    command = "speed 100";
    TelloCLI.write(command.c_str(), command.length());
    delay(100);
    Command = true;
    digitalWrite(18,LOW);
    digitalWrite(17,HIGH);
    digitalWrite(16,LOW);
	}
	else
	{
		if (!digitalRead(25))
		{
      String command = "rc 0 0 0 0";
      TelloCLI.write(command.c_str(), command.length());
      delay(30);
      if (takeoff) {
        command = "land";
        takeoff = false;
      } else {
        command = "takeoff";
        takeoff = true;
      }
      TelloCLI.write(command.c_str(), command.length());
		}
		else if (!digitalRead(33) && takeoff)
		{
			String command = "flip ";
			command += flipside;
			TelloCLI.write(command.c_str(), command.length());
		}
		else if(takeoff)
		{
			double eje_x = sensor.getAccelX()*-1;
			double eje_y = sensor.getAccelY()*-1;

      float pitch = 0;
      float roll = 0;
      float throttle = 0;

      if(!digitalRead(15)) {
        throttle -= 40;
      }
      if(!digitalRead(4)) {
        throttle += 40;
      }

			if (eje_x > 0.35 || eje_x < -0.35)
			{
				//Direccion x positiva: hacia la derecha, negativa: hacia la izquierda
        pitch = eje_x * 60;
				if(eje_x > 0){
					flipside = 'f';
				}
				else
				{
					flipside = 'b';
				}
			}
			if (eje_y > 0.35 || eje_y < -0.35)
			{
        roll = eje_y * 60;
				if(eje_y > 0){
					flipside = 'r';
				}
				else
				{
					flipside = 'l';
				}
			}
			String command = "rc " + String(roll) + " " + String(pitch) + " " + String(throttle) + " 0";
      Serial.println(command);
			TelloCLI.write(command.c_str(), command.length());
			commandSent = true;
		}
		if (commandSent)
		{
			commandSent = false;
			delay(delvar);
		}
	}
}
