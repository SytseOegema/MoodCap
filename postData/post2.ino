#include <Arduino.h>

#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"

#include <HTTPClient.h>

#define USE_SERIAL Serial
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

String cameraName = "ESP32 test Cam1";
String formBoundary = "----MoodCap";


struct FormData {
    char *content;
    size_t length;
};

const char* ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n" \
"A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n" \
"Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n" \
"MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n" \
"A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n" \
"hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n" \
"v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n" \
"eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n" \
"tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n" \
"C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n" \
"zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n" \
"mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n" \
"V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n" \
"bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n" \
"3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n" \
"J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n" \
"291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n" \
"ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n" \
"AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n" \
"TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n" \
"-----END CERTIFICATE-----\n";


void setup() {
    // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    USE_SERIAL.begin(115200);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFi.begin("iPhone van Sytse", "hoihoihoi");

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // init with high specs to pre-allocate larger buffers
    if(psramFound()){
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 10;  //0-63 lower number means higher quality
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_CIF;
        config.jpeg_quality = 12;  //0-63 lower number means higher quality
        config.fb_count = 1;
    }

    USE_SERIAL.print("Almlost the end of setup()");

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        USE_SERIAL.printf("Camera init failed with error 0x%x", err);
        delay(1000);
        ESP.restart();
    }
    USE_SERIAL.print("The end of setup()");
}

void loop() {
    USE_SERIAL.print("start of loop()");

    // wait for WiFi connection
    if((WiFi.status() != WL_CONNECTED)) {
        HTTPClient http;

        camera_fb_t *fb = getCameraContent();

        char *body = getRequestBody(fb);

        USE_SERIAL.print("[HTTP] begin...\n");
        http.begin("https://us-central1-driven-era-310811.cloudfunctions.net/image-upload", ca); //HTTP
        http.addHeader("Content-Type", "multipart/form-data; boundary=" + formBoundary);

        USE_SERIAL.print("[HTTP] POST...\n");
        USE_SERIAL.print(body);

        int httpCode = http.POST(body);

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    } else {
        USE_SERIAL.print("wifi not running\n");
    }

    USE_SERIAL.print("end of loop()");
    delay(15000);
}

camera_fb_t *getCameraContent() {
    USE_SERIAL.print("getCameraContent()");

    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
    if(!fb) {
        Serial.println("Camera capture failed");
        delay(1000);
        ESP.restart();
    }

    return fb;
}

char *getRequestBody(camera_fb_t *fb) {
    USE_SERIAL.print("getRequestBody()");

    FormData header = getRequestHeader();
    FormData tail = getRequestTail();
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;

    USE_SERIAL.print("\nbefore copying to single buffer\n");
    // -1 to remove 1 \0 byte of tail and header

    USE_SERIAL.printf("buffer size: %d", fbLen + tail.length + header.length);
    char buffer[fbLen + tail.length + header.length - 1];
    USE_SERIAL.print("in between\n");
    sprintf(buffer, "%s%d%s", header.content, (char *) fbBuf, tail.content);

    USE_SERIAL.print("after copying to single buffer\n");
    return buffer;
}

FormData toFormData(String value) {
    USE_SERIAL.print("toFormData()");

    size_t len = value.length();
    char buffer[len + 1];
    value.toCharArray(buffer, len + 1);

    return (FormData) {buffer, len};
}

/**
* Returns the beginning of the form including the CameraName and the form header
* part for the image that is send.
*/
FormData getRequestHeader() {
    USE_SERIAL.print("getRequestHeader()");

    // camera name header
    String cameraNameHeader = "--" + formBoundary + "\r\n";
    cameraNameHeader += "Content-Disposition: form-data; name=\"CameraName\"\r\n\r\n";
    cameraNameHeader += cameraName + "\r\n";

    // request header
    String requestHead = "--" + formBoundary + "\r\n";
    requestHead += "Content-Disposition: form-data; name=\"Content\"; filename=\"image.jpeg\"\r\n";
    requestHead += "Content-Type: image/jpeg\r\n\r\n";

    return toFormData(cameraNameHeader + requestHead);
}

/**
* Returns the tail of form. (e.g. closing tag of the form)
*/
FormData getRequestTail() {
    USE_SERIAL.print("getRequestTail()");

    return toFormData("\r\n--" + formBoundary + "--\r\n\r\n");
}
