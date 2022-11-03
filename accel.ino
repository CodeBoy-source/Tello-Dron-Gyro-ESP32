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

float pitch,roll,yaw;
long gyro_x_cal, gyro_y_cal, gyro_z_cal;
int gyro_x, gyro_y, gyro_z;

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
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

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(0);
  roll = 0;
  pitch = 0;
  yaw = 0;
  delvar = 50;
  Connected = false;
  Command = false;
  commandSent = false;
  takeoff = false;
  flipside = 'f';  //Front

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
  Serial.println("INICIALIZANDO");

}

void loop() {
  sensor.read();

  float _pitch = sensor.getPitch();
  float _roll = sensor.getRoll();
  float _yaw = sensor.getYaw();


  pitch = constrain(0.9*pitch+0.1*_pitch, -100, 100);
  roll = constrain(0.9*roll + 0.1*_roll, -100, 100);
  _yaw = int(_yaw)/20 * 20;
  yaw = constrain(0.9*yaw+0.1*_yaw, -100, 100);
  yaw = yaw * -1;

  Serial.println("P: " + String(pitch) + " R: " + String(roll) + " Y: " + String(yaw));
  if (!Connected) {
    String command = "TELLO-5ACC7D";
    char SSID[65];
    strcpy(SSID, command.c_str());
    WiFi.begin(SSID);
  }else if(Connected && !Command){
    String command = "command";
    TelloCLI.write(command.c_str(), command.length());
    delay(100);
    command = "speed 10";
    TelloCLI.write(command.c_str(), command.length());
    delay(100);
    Command = true;
  } else {
    if (touchRead(13) <= 30) {
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
    } else if (touchRead(33) <= 30 && takeoff) {
      String command = "flip ";
      command += flipside;
      TelloCLI.write(command.c_str(), command.length());
    } else if (takeoff) {
      float throttle = 0;

      if (touchRead(32) <= 30) {
        throttle -= 40;
      }
      if (touchRead(4) <= 30) {
        throttle += 40;
      }
      if (roll > 0) {
        flipside = 'r';
      } else {
        flipside = 'l';
      }

      if (pitch > 0) {
        flipside = 'f';
      } else {
        flipside = 'b';
      }
      String command = "rc " + String(roll) + " " + String(pitch) + " " + String(throttle) + " " + String(yaw);
      TelloCLI.write(command.c_str(), command.length());
      commandSent = true;
    }
    if (commandSent) {
      commandSent = false;
      delay(delvar);
    }
  }
}
