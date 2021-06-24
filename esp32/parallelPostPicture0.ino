/*
 * This program send one image via HTTP request to a server each TIMER_INTERVAL.
 *
 * This code is based on the example of Rui Santos. His complete project can be
 * found here:
 * https://RandomNerdTutorials.com/esp32-cam-post-image-photo-server/
*/

#include <Arduino.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"

#define TIMER_INTERVAL 10000
#define NUM_IMAGES 20

// const char* ssid = "iPhone van Sytse";
// const char* password = "hoihoihoi";

const char* ssid = "Ziggo4394718";
const char* password = "rgg7hqptQss3";

String cameraName = "Mood Cap v1";
String boundary = "----MoodCap-esp32";


// framesize_t framesize = FRAMESIZE_XGA; // setting for 10 images within 10 sec
framesize_t framesize = FRAMESIZE_SVGA; // setting for 20 images within 10 sec
int jpegQuality = 2; // number between 0-63 (lower higher quality)
int fbCount = 2;

String serverName = "us-central1-driven-era-310811.cloudfunctions.net";
String serverPath = "/images-upload";
const int serverPort = 80;

WiFiClient client;

// CAMERA_MODEL_AI_THINKER
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

unsigned long frameDelay = 0;   // last time image was sent
QueueHandle_t queue;

typedef struct {
  size_t buf_len;
  uint8_t *buf;
} image;

String requestFormCameraName = "--" + boundary + "\r\n"
  + "Content-Disposition: form-data; name=\"CameraName\"\r\n\r\n"
  + cameraName + "\r\n";

String requestFormTail = "--" + boundary + "--\r\n";


/*
 * Helper functions
*/
void initializeWifi() {
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
}

void initializeCamera() {
  camera_config_t config;

  config.frame_size = framesize;
  config.jpeg_quality = jpegQuality;
  config.fb_count = fbCount;

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

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
}

void startSendImagesTask() {
  xTaskCreate(
    sendImagesTask, /* Task function. */
    "sendImagesTask", /* name of task. */
    20000, /* Stack size of the task */
    NULL, /* parameter of the task */
    1, /* priority of the task */
    NULL); /* Task handle to keep track of created task */
}


image *createImageBuffer(size_t bufferSize)
{
  image *imagePntr = (image*) ps_malloc(sizeof(image));
  if (imagePntr == NULL) {
    Serial.println("[createImageBuffer] Memory error: malloc of image failed");
    ESP.restart();
  }

  imagePntr->buf_len = bufferSize;

  imagePntr->buf = (uint8_t*) ps_malloc(sizeof(uint8_t) * bufferSize);
  if (imagePntr->buf == NULL) {
    Serial.println("[createImageBuffer] Memory error: malloc of buf for image failed");
    ESP.restart();
  }

  return imagePntr;
}

void freeImage(image *imagePntr) {
  free(imagePntr->buf);
  free(imagePntr);
}

image *captureImage()
{
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }

  image *im = createImageBuffer(fb->len);
  memcpy(im->buf, fb->buf, sizeof(uint8_t) * fb->len);

  esp_camera_fb_return(fb);

  return im;
}

String generateRequestFormImageHeader(int num) {
  // request header
  String requestHead = "--" + boundary + "\r\n";
  requestHead += "Content-Disposition: form-data; name=\"Content";
  requestHead += String(num) + "\"; filename=\"image" + String(num) + ".jpeg\"\r\n";
  requestHead += "Content-Type: image/jpeg\r\n\r\n";

  return requestHead;
}

unsigned int computeRequestContentSize(image **images,
  size_t imagesLength)
{
  unsigned int contentSize = requestFormCameraName.length();
  contentSize += requestFormTail.length();
  // Newline that comes after all image data
  String newLine = "\r\n";
  contentSize += newLine.length() * imagesLength;

  // compute total length for all headers of each images.
  size_t increment = 1;
  size_t numImages = imagesLength;

  do {
    String temp = generateRequestFormImageHeader(increment);
    increment *= 10;
    if (numImages > increment) {
      contentSize += temp.length() * increment;
      numImages -= increment;
    } else {
      contentSize += temp.length() * numImages;
    }
  } while(increment < imagesLength);

  for (size_t idx = 0; idx < imagesLength; idx++)
  {
    contentSize += images[idx]->buf_len;
  }

  return contentSize;
}

void sendImagesTask(void *parameter) {
  image *images[NUM_IMAGES];

  for(;;) {
    for(size_t idx = 0; idx < NUM_IMAGES;) {
      BaseType_t success;
      success = xQueueReceive(queue, &images[idx], 100 * portTICK_PERIOD_MS);
      if(success == pdTRUE) {
        Serial.printf("Receive image %d for request.\n", idx);
        idx++;
      }
    }

    sendHTTPRequest(images, NUM_IMAGES);
  }
}

void sendHTTPRequest(image **images, size_t imagesLength) {
  String getAll, getBody;
  WiFiClient client;
  unsigned int contentSize = computeRequestContentSize(images, imagesLength);

  unsigned long beforeRequest = millis();

  Serial.println("Connecting to server: " + serverName);
  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");
    /*
     * Start of actually sending the request
     */
    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(contentSize));
    client.println("Content-Type: multipart/form-data; boundary=" + boundary);
    client.println();

    // send HTTP beginning of data
    client.print(requestFormCameraName);

    for (size_t i = 0; i < imagesLength; i++) {
      client.print(generateRequestFormImageHeader(i));
      uint8_t *fbBuf = images[i]->buf;
      size_t fbLen = images[i]->buf_len;
      for (size_t n = 0; n < fbLen; n = n + 1024) {
        if (n + 1024 < fbLen) {
          client.write(fbBuf, 1024);
          fbBuf += 1024;
        } else if (fbLen % 1024 > 0) {
          size_t remainder = fbLen % 1024;
          client.write(fbBuf, remainder);
        }
      }
      freeImage(images[i]);
      client.print("\r\n");
    }

    client.print(requestFormTail);
    /*
     * End of sending the request
     */

    unsigned long requestTime = millis() - beforeRequest;

    Serial.printf("Time required to send request: %d\n", requestTime);

    int timoutTimer = 10000;
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
  }  else {
    getBody = "Connection to " + serverName + " failed.";
    Serial.println(getBody);
  }
}

/*
 * main functions (setup and loop)
*/
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);

  initializeWifi();
  initializeCamera();
  queue = xQueueCreate(NUM_IMAGES, sizeof(image*));
  startSendImagesTask();

  frameDelay = TIMER_INTERVAL / NUM_IMAGES;
}

void loop() {
  delay(frameDelay);
  image *currentImage = captureImage();
  xQueueSend(queue, &currentImage, portMAX_DELAY);
}
