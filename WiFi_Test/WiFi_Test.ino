#include "WiFiS3.h"
#include "arduino_secrets.h"
#include <C305_Steam_Air_GDK101_GAMMA.h> // GDK-101 라이브러리

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiServer server(80);
SteamGDK101 gdk101; // GDK-101 인스턴스

void setup() {
  Serial.begin(115200);
  delay(10000); // 시리얼 통신 안정화 대기
  Serial.println("Setup 시작");

  // GDK-101 초기화
  gdk101.begin();
  Serial.println("GDK-101 초기화 완료");

  // Wi-Fi 연결
  Serial.println("Wi-Fi 연결 시도 중...");
  WiFi.begin(ssid, pass);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) { // 20초 동안 시도
    delay(1000);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi 연결 성공!");
    Serial.print("IP 주소: ");
    Serial.println(WiFi.localIP());
    server.begin(); // 웹 서버 시작
  } else {
    Serial.println("\nWi-Fi 연결 실패!");
    while (true); // 무한 루프로 멈춤
  }
}



void loop() {
  gdk101.read(); // 센서 데이터 읽기

  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    // JSON API 요청 처리
    if (request.indexOf("/api/sensor") != -1) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.print("{");
      client.print("\"10min_avg\": ");
      client.print(gdk101.get10minAvg());
      client.print(", \"1min_avg\": ");
      client.print(gdk101.get1minAvg());
      client.print(", \"time_min\": ");
      client.print(gdk101.getMeasuringTimeMin());
      client.print(", \"time_sec\": ");
      client.print(gdk101.getMeasuringTimeSec());
      client.println("}");
      client.stop();
      return;
    }

    // HTML 페이지 요청 처리
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head>");
    client.println("<meta charset=\"UTF-8\">");
    client.println("<title>GDK-101 감마선 센서</title>");
    client.println("<script>");
    client.println("setInterval(async function() {");
    client.println("  const response = await fetch('/api/sensor');");
    client.println("  const data = await response.json();");
    client.println("  document.getElementById('10min_avg').innerText = data['10min_avg'] + ' µSv/hr';");
    client.println("  document.getElementById('1min_avg').innerText = data['1min_avg'] + ' µSv/hr';");
    client.println("  document.getElementById('time').innerText = data['time_min'] + '분 ' + data['time_sec'] + '초';");
    client.println("}, 2000);");
    client.println("</script>");
    client.println("</head>");
    client.println("<body>");
    client.println("<h1>GDK-101 감마선 센서 데이터</h1>");
    client.println("<p>10분 평균값: <span id='10min_avg'>로딩 중...</span></p>");
    client.println("<p>1분 평균값: <span id='1min_avg'>로딩 중...</span></p>");
    client.println("<p>측정 시간: <span id='time'>로딩 중...</span></p>");
    client.println("</body></html>");
    client.stop();
  }
}
