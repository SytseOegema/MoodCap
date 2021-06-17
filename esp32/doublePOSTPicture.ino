#include <Arduino.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM - 1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define NUM_IMAGES 2

typedef struct {
  size_t buf_len;
  uint8_t *buf;
} JPEG;

JPEG images[NUM_IMAGES];

const char *ssid = "YourWiFiNetwork";
const char *password = "YourWiFiPassword";

String boundary = "----moodcap-esp32-cam1";
String serverName = "us-central1-driven-era-310811.cloudfunctions.net";
String serverPath = "/images-upload";
String cameraName = "ESP32-mood-cap";
const int serverPort = 80;

WiFiClient client;

const int timerInterval = 30000; // time between each HTTP POST image
unsigned long previousMillis = 0; // last time image was sent

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());

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
  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 2; //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 2; //0-63 lower number means higher quality
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init( & config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
}

/*
 * Infinite loop that runs sendImages() every timerInterval
 */
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= timerInterval) {

    for (size_t i = 0; i < NUM_IMAGES; i++) {
      images[i] = captureImage();
    }

    sendImages();
    previousMillis = currentMillis;
  }
}

JPEG captureImage() {
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }

  JPEG image = { fb->len, fb->buf };
  esp_camera_fb_return(fb);

  return image;
}

/*
 * Send HTTP request with the image and the name of the camera
 */
String sendImages() {
  String getAll;
  String getBody;

  Serial.println("Connecting to server: " + serverName);

  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");

    String formCameraName = generateRequestFormCameraName();
    // only digits 0 to 9 are used, thus the size of the string is the same.
    String formImageHeader = generateRequestFormImageHeader(0);
    String tail = "\r\n--" + boundary + "--\r\n";

    // compute HTTP content length
    uint32_t extraLen = formCameraName.length() + tail.length();
    extraLen = extraLen + formImageHeader.length() * NUM_IMAGES;
    uint32_t imageLen = 0;

    for (size_t i = 0; i < NUM_IMAGES; i++) {
      imageLen += images[i].buf_len;
    }

    uint32_t totalLen = imageLen + extraLen;

    /*
     * Start of actually sending the request
     */
    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=" + boundary);
    client.println();

    // send HTTP beginning of data
    client.print(formCameraName);

    for (size_t i = 0; i < NUM_IMAGES; i++) {
      uint8_t *fbBuf = images[i].buf;
      size_t fbLen = images[i].buf_len;
      for (size_t n = 0; n < fbLen; n = n + 1024) {
        if (n + 1024 < fbLen) {
          client.write(fbBuf, 1024);
          fbBuf += 1024;
        } else if (fbLen % 1024 > 0) {
          size_t remainder = fbLen % 1024;
          client.write(fbBuf, remainder);
        }
      }
    }

    client.print(tail);
    /*
     * End of sending the request
     */

    int timoutTimer = 5000;
    long startTimer = millis();
    boolean state = false;

    while ((startTimer + timoutTimer) > millis()) {
      Serial.print(".");
      delay(100);
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (getAll.length() == 0) {
            state = true;
          }
          getAll = "";
        } else if (c != '\r') {
          getAll += String(c);
        }
        if (state == true) {
          getBody += String(c);
        }
        startTimer = millis();
      }
      if (getBody.length() > 0) {
        break;
      }
    }
    Serial.println();
    client.stop();
    Serial.println(getBody);
  } else {
    getBody = "Connection to " + serverName + " failed.";
    Serial.println(getBody);
  }
  return getBody;
}

/*
 * Generate head of the data for the HTTP request
 */
String generateRequestFormCameraName() {
  // camera name header
  String cameraNameHeader = "--" + boundary + "\r\n";
  cameraNameHeader += "Content-Disposition: form-data; name=\"CameraName\"\r\n\r\n";
  cameraNameHeader += cameraName + "\r\n";

  return cameraNameHeader;
}

String generateRequestFormImageHeader(int num) {
  // request header
  String requestHead = "--" + boundary + "\r\n";
  requestHead += "Content-Disposition: form-data; name=\"Content" + String(num) + "\"; filename=\"image.jpeg\"\r\n";
  requestHead += "Content-Type: image/jpeg\r\n\r\n";

  return requestHead;
}
