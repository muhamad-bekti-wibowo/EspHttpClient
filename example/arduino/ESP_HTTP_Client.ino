#define SerialHTTPClient
#include <Arduino.h>
#include <WiFi.h>  
#include "EspHttpClient.h"

const char* ssid = "MIFON";
const char* password = "";
const String apiToken = "";

EspHttpClient httpClient;

void setup() {
  Serial.begin(9600);
  delay(100);
  
  // Hubungkan ke WiFi
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi terhubung");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());
  
  // Konfigurasikan client HTTP
  httpClient.setToken(apiToken);
  httpClient.setMaxRedirects(3);
  httpClient.setTimeout(15000); // 15 detik timeout
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Contoh GET request HTTPS
    Serial.println("\nMelakukan permintaan HTTPS GET...");
    String jsonData = "{\"sensor\":\"DHT11\",\"temperature\":25.4,\"humidity\":60}";
        jsonData.replace("\"", "");
    String response = httpClient.get("yourlink");
    Serial.println(response);
    // Contoh POST request HTTP
    //Serial.println("\nMelakukan permintaan HTTP POST...");
    //String jsonPayload = "{\"name\":\"Test Device\",\"value\":123}";
    //String postResponse = httpClient.post("yourlink", jsonPayload);
    //Serial.println(postResponse);
    // Tunggu sebelum permintaan berikutnya
    delay(60000); // 1 menit
  } else {
    Serial.println("WiFi tidak terhubung, menghubungkan kembali...");
    WiFi.begin(ssid, password);
    delay(10000);
  }
}