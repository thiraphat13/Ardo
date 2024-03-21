#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>

const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "8e96b804-1a98-4cfe-b8d1-b2470ccbf9a4";
const char* mqtt_username = "1cLJT2ECvEhwYWqNC11eanXzopQnt2XC";
const char* mqtt_password = "ZQkr3QpEWvsWg2uPnfR57jBZV55gjLVj";
const char* topic_open = "@msg/open";
const char* topic_name = "@msg/sms-0";
const char* nameDv = "QC-1";

int Q_cout = 0;
int maxDv = 10;

WiFiClient espClient;
PubSubClient client(espClient);

#define BUZZER_PIN 33
#define BUTT_PIN 19
#define OLED_RESET 4

Adafruit_SSD1306 display(OLED_RESET);

void disprint(String text, int s) {
  display.clearDisplay();
  display.setTextSize(s);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(text);
  display.display();
}

void disprintname() {
  display.clearDisplay();
  if (Q_cout == 0) {
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(nameDv);
  } else {
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(nameDv);
    display.print("Order : ");
    display.println(Q_cout);
  }
  display.display();
}

void sAlert() {
  tone(BUZZER_PIN, 1000);
  delay(1000);
  tone(BUZZER_PIN, 0);
  delay(500);
}

void setup() {
  Serial.begin(115200);

  WiFiManager wm;

    bool res;
    res = wm.autoConnect("AutoConnectAP");
    disprint("AutoConnectAP", 2);

    if(!res) {
        Serial.println("Failed to connect");
        disprint("Failed to connect", 2);
        ESP.restart();
    } 
    else {  
        Serial.println("connected...yeey :)");
        disprint("connected...yeey :)", 2);
    }

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  pinMode(BUTT_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.clearDisplay();
}


void checkStatus(String t) {
  if (t == "checkStatus") {
    disprint("CheckStatus", 2);
    client.publish(topic_name, "Confirm-CheckStatus");
    sAlert();
    client.publish(topic_name, "Connected");
    client.publish(topic_open, "QC-1 : Confirm-checkStatus");
    disprint("Connected", 2);
  } else if (t == "startStatus") {
    disprint("StartStatus", 2);
    client.publish(topic_name, "Confirm-startStatus");
    sAlert();
    client.publish(topic_name, "Connected");
    client.publish(topic_open, "QC-1 : Confirm-startStatus");
    disprint("Connected", 2);
    Q_cout = 0;
  }
}

void callMessage(String t, String m) {
  if (m == "callMessage") {
    if (t == topic_name) {
      disprint("Call", 2);
      sAlert();
      sAlert();
      client.publish(topic_name, "Confirm-Call");
      client.publish(topic_open, "QC-1 : Confirm-callMessage");
      disprint("Confirm", 2);
      Q_cout = 0;
    } else if (Q_cout == 1) {
      disprint("Call", 2);
      sAlert();
      sAlert();
      client.publish(topic_name, "Confirm-Call");
      client.publish(topic_open, "QC-1 : Confirm-callMessage");
      disprint("Confirm", 2);
      Q_cout = 0;
    } else if (Q_cout != 0) {
      Q_cout--;
    }
  }
}

void cancelMessage(String m) {
  if (m == "cancelMessage") {
    disprint("Cancel", 2);
    sAlert();
    client.publish(topic_name, "Confirm-Cancel");
    client.publish(topic_open, "QC-1 : Confirm-cancelMessage");
    disprint("Confirm", 2);
    Q_cout = 0;
  }
}

void getQueue() {
  client.publish(topic_name, "getQueue");
  Q_cout = 0;
}

void coutQueue(String t, String m) {
  if (t == topic_name && m == "coutQueue") {
    Q_cout++;
  }
}

void reQueue(String m) {
  if (m == "reQueue") {
    getQueue();
  }
}

void loop() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    disprint("Attempting MQTT connection…", 1);
    delay(1000);
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      disprint("connected", 1);
      client.subscribe(topic_name);
      client.subscribe(topic_open);
      client.publish(topic_name, "Connected");
      client.publish(topic_open, "QC-1 : Connected");
    } else {
      Serial.print("failed, rc=");
      disprint("failed", 1);
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
      return;
    }
    disprintname();
  }
  client.loop();

  if (digitalRead(BUTT_PIN) == LOW) {
    getQueue();
    delay(2000);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrive [");
  Serial.print(topic);
  Serial.print("]");
  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);
  checkStatus(message);
  callMessage(topic, message);
  cancelMessage(message);
  coutQueue(topic, message);
  reQueue(message);
  delay(2000);
  disprintname();
}
