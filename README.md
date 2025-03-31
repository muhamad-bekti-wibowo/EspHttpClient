# EspHttpClient

EspHttpClient adalah library HTTP/HTTPS ringan untuk ESP8266 dan ESP32 dengan fitur autentikasi, header kustom, dan pengalihan otomatis.

## Fitur
- Mendukung metode HTTP: `GET`, `POST`, `PUT`, `DELETE`, `PATCH`.
- Menangani **redirect otomatis** dengan batasan maksimal (`MAX_REDIRECTS`).
- Menambahkan **custom headers** untuk kebutuhan otentikasi dan informasi tambahan.
- Dukungan **Bearer Token** untuk autentikasi API.
- Pengaturan **timeout** untuk mengontrol waktu tunggu HTTP request.
- **HTTPS support** menggunakan `WiFiClientSecure`.
- Opsi **debugging** menggunakan `SerialHTTPClient`.

## Instalasi
1. Unduh atau clone repository ini.
2. Tambahkan file `EspHttpClient.h` dan `EspHttpClient.cpp` ke dalam proyek Arduino Anda.
3. Pastikan library ESP8266WiFi atau WiFi.h serta HTTPClient.h tersedia.

## Penggunaan

### Inisialisasi
```cpp
#include "EspHttpClient.h"

EspHttpClient client;
```

### Mengirim Permintaan GET
```cpp
String response = client.get("http://example.com/api/data");
Serial.println(response);
```

### Mengirim Permintaan POST dengan Payload JSON
```cpp
String payload = "{\"key\":\"value\"}";
String response = client.post("http://example.com/api/data", payload);
Serial.println(response);
```

### Menambahkan Header Kustom
```cpp
client.addHeader("Authorization", "Bearer YOUR_ACCESS_TOKEN");
```

## Konfigurasi Tambahan

### Mengatur Timeout
```cpp
client.setTimeout(15000); // 15 detik
```

### Mengaktifkan Pengalihan Otomatis
```cpp
client.setMaxRedirects(3); // Maksimum 3 kali redirect
```

### Menggunakan HTTPS dengan Sertifikat Tidak Terverifikasi (Hanya untuk Pengujian)
```cpp
client.sendRequest("https://example.com", "GET");
```

## Lisensi
Proyek ini dirilis di bawah lisensi MIT. Anda bebas menggunakan dan memodifikasinya sesuai kebutuhan.
