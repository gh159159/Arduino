#include "WiFiS3.h"
#include "arduino_secrets.h"
#include <C305_Steam_Air_GDK101_GAMMA.h> // GDK-101 라이브러리
#include <PubSubClient.h>               // MQTT 라이브러리

// Wi-Fi 정보
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// MQTT 브로커 정보
const char* mqtt_server = "192.168.0.40"; // MQTT 브로커 IP 주소
const int mqtt_port = 1883;
const char* mqtt_topic = "sensor/gdk101"; // 발행할 토픽

WiFiClient espClient;
PubSubClient client(espClient);

SteamGDK101 gdk101; // GDK-101 인스턴스

void setup_wifi() {
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // MQTT 브로커 연결
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("GDK101Client")) { // 클라이언트 ID
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(10000); // 시리얼 통신 안정화 대기
  Serial.println("Setup 시작");

  // GDK-101 초기화
  gdk101.begin();
  Serial.println("GDK-101 초기화 완료");

  // Wi-Fi 연결
  setup_wifi();

  // MQTT 브로커 설정
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect(); // MQTT 브로커 재연결
  }
  client.loop();

  // 센서 데이터 읽기
  gdk101.read();
  float avg10min = gdk101.get10minAvg();
  float avg1min = gdk101.get1minAvg();
  int time_min = gdk101.getMeasuringTimeMin();
  int time_sec = gdk101.getMeasuringTimeSec();

  // JSON 형태로 데이터 구성
  String payload = "{";
  payload += "\"10min_avg\": " + String(avg10min, 2) + ",";
  payload += "\"1min_avg\": " + String(avg1min, 2) + ",";
  payload += "\"time_min\": " + String(time_min) + ",";
  payload += "\"time_sec\": " + String(time_sec);
  payload += "}";

  // MQTT 메시지 발행
  client.publish(mqtt_topic, payload.c_str());
  Serial.println("Published MQTT message: " + payload);

  delay(2000); // 2초 간격으로 데이터 전송
}
