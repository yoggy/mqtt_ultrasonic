//
// mqtt_ultrasonic.ino - publish the distance that was measured using a HC-SR04.
//
// sensor & device
//    https://www.switch-science.com/catalog/2860/
//    http://akizukidenshi.com/catalog/g/gM-09109/
//
// github:
//    https://github.com/yoggy/mqtt_ultrasonic
//
// license:
//     Copyright (c) 2016 yoggy <yoggy0@gmail.com>
//     Released under the MIT license
//     http://opensource.org/licenses/mit-license.php;
//
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/
#include <Wire.h>

char *wifi_ssid = "wifi_ssid";
char *wifi_password = "wifi_password";

char *mqtt_server    = "mqtt.example.com";
int  mqtt_port      = 1883;

char *mqtt_username = "username";
char *mqtt_password = "password";

char *publish_topic = "dev/ultrasonic/1";

#define PIN_LED  0
#define PIN_TRIG 12
#define PIN_ECHO 13

WiFiClient wifi_client;
PubSubClient mqtt_client(mqtt_server, mqtt_port, NULL, wifi_client);

void setup() {
  Wire.begin(4, 5); // scl/sda
  aqm0802_init();
  
  Serial.begin(9600);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  int wifi_count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (wifi_count % 2 == 0) digitalWrite(PIN_LED, HIGH);
    else                     digitalWrite(PIN_LED, LOW);
    wifi_count ++;
    delay(500);
  }

  if (mqtt_client.connect("client_id", mqtt_username, mqtt_password) == false) {
    Serial.println("conn failed...  ");
    delay(1000);
    reboot();
  }
  Serial.println("connected!");
}
 
void loop() {
  int d = (int)measure_distance();

  display_lcd_display(d);
  publish_distance(d);

  delay(20);

  // for MQTT
  if (!mqtt_client.connected()) {
    reboot();
  }
  mqtt_client.loop();
}

void publish_distance(int d) {
  char buf[256];

  snprintf(buf, 256, "{\"distance\":%d, ,\"unit\":\"cm\", \"tick\":%d}", d, millis());
  mqtt_client.publish(publish_topic, buf);
  Serial.println(d);
}

void display_lcd_display(int d) {
  char buf[17];
  snprintf(buf, 17, "distance%d cm    ", d);
  aqm0802_print(buf);
}

float measure_distance() {
  float distance = 0;

  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(10);

  digitalWrite(PIN_LED, HIGH);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(11);

  digitalWrite(PIN_TRIG, LOW);

  unsigned long dulation = pulseIn(PIN_ECHO, HIGH, 80000);
  if (dulation > 0) {
    distance = dulation / 2;
    distance = distance * 340 * 100 / 1000000; // ultrasonic speed is 340m/s = 34000cm/s = 0.034cm/us 
  }
  else {
    distance = 0.0f;
  }
  digitalWrite(PIN_LED, LOW);

  return distance;
}

// reboot for ESP8266
void reboot() {
  for ( int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      digitalWrite(PIN_LED, LOW);
      delay(100);
      digitalWrite(PIN_LED, HIGH);
      delay(100);
    }
  }
  ESP.reset();
  while (true) {};
}
