/*library ESP HTTP Client yang dirancang untuk memudahkan pengiriman permintaan HTTP / HTTPS
 *dari perangkat berbasis ESP8266 dan ESP32. Library ini mendukung berbagai metode HTTP seperti
 *GET, POST, PUT, DELETE, dan PATCH, serta dapat menangani pengalihan(redirect)
 *secara otomatis hingga batas tertentu.
 *Created by Claude And Command Prompt, Editing by Bekti, March 31, 2025
 */

#ifndef ESP_HTTP_CLIENT_H
#define ESP_HTTP_CLIENT_H

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 
#endif
#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#endif
#include <WiFiClientSecure.h>

#define MAX_REDIRECTS 5
#define DEFAULT_TIMEOUT 10000 // 10 detik
#define MAX_HEADERS 10        // Menambahkan definisi MAX_HEADERS

class EspHttpClient
{
private:
    String _token;
    int _timeout;
    int _maxRedirects;
    bool _preserveMethod; // Apakah mempertahankan metode HTTP saat redirect (POST->POST) atau mengikuti standar (POST->GET)

    // Menambahkan struktur untuk header kustom
    struct Header
    {
        String name;
        String value;
    };

    // Menambahkan variabel untuk header kustom
    Header _customHeaders[MAX_HEADERS];
    int _headerCount;

    // Fungsi untuk mengekstrak URL dari respons HTML redirect
    String extractUrlFromHtml(const String &html)
    {
        int hrefPos = html.indexOf("HREF=\"");
        if (hrefPos == -1)
        {
            hrefPos = html.indexOf("href=\"");
        }

        if (hrefPos != -1)
        {
            int startPos = hrefPos + 6; // Panjang "HREF=\"" atau "href=\""
            int endPos = html.indexOf("\"", startPos);

            if (endPos != -1)
            {
                String url = html.substring(startPos, endPos);
                // Mengkonversi entity HTML seperti &amp; menjadi &
                url.replace("&amp;", "&");
                return url;
            }
        }

        return "";
    }

    // Cek apakah respons HTML mengindikasikan pengalihan
    bool isHtmlRedirect(const String &html)
    {
        return html.indexOf("<HTML>") != -1 &&
               (html.indexOf("Moved Temporarily") != -1 ||
                html.indexOf("Moved Permanently") != -1 ||
                html.indexOf("Found") != -1 ||
                html.indexOf("See Other") != -1 ||
                html.indexOf("Redirect") != -1);
    }

    // Fungsi internal untuk memproses respons dan mengikuti pengalihan jika ada
    String handleRequest(HTTPClient &http, WiFiClient &client, const String &method, const String &payload, int &httpCode, int redirectCount = 0)
    {
        // Kirim permintaan sesuai metode
        if (method.equals("GET"))
        {
            httpCode = http.GET();
        }
        else if (method.equals("POST"))
        {
            httpCode = http.POST(payload);
        }
        else if (method.equals("PUT"))
        {
            httpCode = http.PUT(payload);
        }
        else if (method.equals("DELETE"))
        {
            httpCode = http.sendRequest("DELETE", payload);
        }
        else if (method.equals("PATCH"))
        {
            httpCode = http.sendRequest("PATCH", payload);
        }
        else
        {
            httpCode = http.sendRequest(method.c_str(), payload);
        }

        String responsePayload = "";

        if (httpCode > 0)
        {
            responsePayload = http.getString();

            // Kode respons 3xx berarti pengalihan
            if ((httpCode >= 300 && httpCode < 400) || isHtmlRedirect(responsePayload))
            {
                if (redirectCount >= _maxRedirects)
                {
#ifdef SerialHTTPClient
                    Serial.println("Terlalu banyak pengalihan, mencapai batas maksimum");
#endif
                    return responsePayload;
                }

                // Coba mendapatkan header Location untuk pengalihan
                String newUrl = http.header("Location");

                // Jika tidak ada header Location, coba ekstrak dari HTML
                if (newUrl.length() == 0 && isHtmlRedirect(responsePayload))
                {
                    newUrl = extractUrlFromHtml(responsePayload);
                }

                if (newUrl.length() > 0)
                {
#ifdef SerialHTTPClient
                    Serial.print("Mengikuti pengalihan ke: ");
                    Serial.println(newUrl);
#endif
                    http.end();
                    client.stop();

                    // Menentukan metode untuk permintaan redirect
                    String redirectMethod;

                    // Mengikuti spesifikasi HTTP: 301, 302, 303 mengubah POST/PUT/DELETE menjadi GET
                    // Kecuali 307, 308 yang mempertahankan metode asli
                    if (_preserveMethod || httpCode == 307 || httpCode == 308)
                    {
                        redirectMethod = method; // Pertahankan metode asli
                    }
                    else
                    {
                        redirectMethod = "GET"; // Standar HTTP: ubah ke GET
                    }

#ifdef SerialHTTPClient
                    Serial.print("Metode asli: ");
                    Serial.print(method);
                    Serial.print(", Metode redirect: ");
                    Serial.println(redirectMethod);
#endif

                    // Kirim permintaan baru ke URL redirect
                    return sendRequest(newUrl, redirectMethod, redirectMethod == "GET" ? "" : payload, redirectCount + 1);
                }
            }

            if (redirectCount == 0)
            {
#ifdef SerialHTTPClient
                Serial.println("Respons diterima:");
                Serial.println(responsePayload.substring(0, 500)); // Tampilkan 500 karakter pertama saja untuk menghemat memori
                if (responsePayload.length() > 500)
                {
                    Serial.println("... (respons terpotong)");
                }
#endif
            }
        }
        else
        {
#ifdef SerialHTTPClient
            Serial.print("Permintaan HTTP gagal, error: ");
            Serial.println(http.errorToString(httpCode));
#endif
        }
        return responsePayload;
    }

public:
    EspHttpClient(String token = "", int timeout = DEFAULT_TIMEOUT, int maxRedirects = MAX_REDIRECTS)
    {
        _token = token;
        _timeout = timeout;
        _maxRedirects = maxRedirects;
        _preserveMethod = false; // Default: ikuti standar HTTP (POST/PUT/DELETE -> GET saat redirect)
        _headerCount = 0;        // Inisialisasi _headerCount
    }

    // Mengatur token otentikasi
    void setToken(String token)
    {
        _token = token;
    }

    // Mengatur timeout permintaan dalam milidetik
    void setTimeout(int timeout)
    {
        _timeout = timeout;
    }

    // Mengatur jumlah maksimum pengalihan yang akan diikuti
    void setMaxRedirects(int maxRedirects)
    {
        _maxRedirects = maxRedirects;
    }

    // Mengatur apakah akan mempertahankan metode HTTP saat redirect
    void setPreserveMethod(bool preserve)
    {
        _preserveMethod = preserve;
    }

    // Menambahkan header kustom
    void addHeader(const String &name, const String &value)
    {
        _customHeaders[_headerCount].name = name;
        _customHeaders[_headerCount].value = value;
        _headerCount = min(_headerCount + 1, MAX_HEADERS);
    }

    // Mengirim permintaan HTTP/HTTPS dengan metode, payload, dan menangani pengalihan
    String sendRequest(const String &url, const String &method = "GET", const String &payload = "", int redirectCount = 0)
    {

#ifdef SerialHTTPClient
        Serial.print("Free heap sebelum permintaan: ");
        Serial.println(ESP.getFreeHeap());
        Serial.print("URL: ");
        Serial.println(url);
        Serial.print("Metode: ");
        Serial.println(method);
        if (payload.length() > 0)
        {
            Serial.print("Payload: ");
            Serial.println(payload);

#endif
        }

        if (WiFi.status() != WL_CONNECTED)
        {

#ifdef SerialHTTPClient
            Serial.println("WiFi tidak terhubung, membatalkan permintaan.");
#endif

            return "";
        }

        HTTPClient http;
        int httpCode = 0;
        String responsePayload = "";
        bool isHttps = url.startsWith("https://");

        if (isHttps)
        {
            // Untuk HTTPS
            WiFiClientSecure secureClient;
            secureClient.setInsecure(); // Abaikan verifikasi sertifikat SSL (hanya untuk pengujian)

            http.begin(secureClient, url);
            http.setTimeout(_timeout);

            // Tambahkan header default
            http.addHeader("Content-Type", "application/json");
            if (_token.length() > 0)
            {
                http.addHeader("Authorization", "Bearer " + _token);
            }

            // Tambahkan header kustom
            for (int i = 0; i < _headerCount; i++)
            {
                http.addHeader(_customHeaders[i].name, _customHeaders[i].value);
            }

            // Proses permintaan
            responsePayload = handleRequest(http, secureClient, method, payload, httpCode, redirectCount);

            http.end();
            secureClient.stop();
        }
        else
        {
            // Untuk HTTP
            WiFiClient client;

            http.begin(client, url);
            http.setTimeout(_timeout);

            // Tambahkan header default
            http.addHeader("Content-Type", "application/json");
            if (_token.length() > 0)
            {
                http.addHeader("Authorization", "Bearer " + _token);
            }

            // Tambahkan header kustom
            for (int i = 0; i < _headerCount; i++)
            {
                http.addHeader(_customHeaders[i].name, _customHeaders[i].value);
            }

            // Proses permintaan
            responsePayload = handleRequest(http, client, method, payload, httpCode, redirectCount);

            http.end();
            client.stop();
        }

        if (redirectCount == 0)
        {
#ifdef SerialHTTPClient
            Serial.print("HTTP Status Code: ");
            Serial.println(httpCode);
            Serial.print("Free heap setelah permintaan: ");
            Serial.println(ESP.getFreeHeap());
#endif
        }

        return responsePayload;
    }

    // Metode khusus untuk permintaan GET
    String get(const String &url)
    {
        return sendRequest(url, "GET");
    }

    // Metode khusus untuk permintaan POST
    String post(const String &url, const String &payload)
    {
        return sendRequest(url, "POST", payload);
    }

    // Metode khusus untuk permintaan PUT
    String put(const String &url, const String &payload)
    {
        return sendRequest(url, "PUT", payload);
    }

    // Metode khusus untuk permintaan DELETE
    String del(const String &url)
    {
        return sendRequest(url, "DELETE");
    }

    // Metode khusus untuk permintaan PATCH
    String patch(const String &url, const String &payload)
    {
        return sendRequest(url, "PATCH", payload);
    }
};

#endif // ESP_HTTP_CLIENT_H