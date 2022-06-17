#include "WiFi.h"
#include "FastLED.h"

const char *Ssid = "ESP32_S3_Server";
const char *PassWord = "SaligiaYZK";
const int LocalPort = 1688;

/************************************************************************************
 * 函数: taskRGB()
 * 作用：用于控制RGB灯珠
 * 备注：（1）本开发板使用WS2812B，为无时钟线的RGB灯珠；
 *      （2）详细设置见https://github.com/FastLED/FastLED/blob/master
************************************************************************************/
void taskRGB(void *arg) {
  const int ledPin = 48;
  const int ledNum = 1;
  CRGB leds[ledNum];
  FastLED.addLeds<NEOPIXEL, ledPin>(leds, ledNum);
  FastLED.setBrightness(64);

  const TickType_t xFreq = 500;   // 使用绝对任务延时
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;) {
    leds[0] = CRGB::Red;FastLED.show();vTaskDelayUntil(&xLastWakeTime, xFreq);
    leds[0] = CRGB::Green;FastLED.show();vTaskDelayUntil(&xLastWakeTime, xFreq);
    leds[0] = CRGB::Blue;FastLED.show();vTaskDelayUntil(&xLastWakeTime, xFreq);
  }
}

/************************************************************************************
 * 函数: taskTCP()
 * 作用：用于连接其他客户端，并实现与客户端的通讯
 * 待解决问题：目前只能连接一个设备，同时只接受一个字符
************************************************************************************/
void taskTCP(void *arg) {
  // 创建WIFI热点
  while (!WiFi.softAP(Ssid, PassWord)) {
    Serial.println("Retrying...");
    vTaskDelay(500);
  }
  Serial.println("WiFi established!");

  // 建立TCP服务器
  WiFiServer server(LocalPort);
  server.begin();
  Serial.println("TCP Server started!");
  Serial.print("Access point: ");Serial.println(Ssid);
  Serial.print("IP address: ");Serial.println(WiFi.softAPIP());

  // 连接客户端并处理相关工作
  // 目前只连接一个客户端，每次只能读取一个字符
  for (;;) {
    WiFiClient client = server.available();
    if (client) {
      client.write("Connection to ESP32-S3 is established!\r\n");
      Serial.println("Client connected!");
      while (client.connected()) {
        if (client.available()) {
//          char tmp[30];
//          Serial.println(client.readString());
//          String recvData = client.readString();
//          recvData.toCharArray(tmp, recvData.length());
//          Serial.print("Get info from client: ");Serial.println(tmp);
//          client.print("Get info: ");client.println(tmp);

            char c = client.read();
            Serial.print("Get info from client: ");Serial.println(c);
            client.print("Get info: ");client.println(c);
        }
      }
      client.stop();
      Serial.println("Client disconnected!");
    }
  }
}

/************************************************************************************
 * 函数: deadLoop()
 * 作用：用于验证Core0上存在于IDLE(0)进程中的看门狗
************************************************************************************/
void deadLoop(void *arg) {
  for (;;) {
  }
}

void setup() {
  Serial.begin(115200);
  Serial.print("Chip model = ");Serial.println(ESP.getChipModel());
  Serial.print("Avaliable cores: ");Serial.println(ESP.getChipCores());
  Serial.println(" ");

  // 创建网络服务器
  xTaskCreate(taskTCP, "TCP Server", 1024 * 4, nullptr, 2, nullptr);

  // RGB灯闪烁
  xTaskCreate(taskRGB, "RGB Blink", 1024 * 2, nullptr, 2, nullptr);

  // 验证看门狗
  // xTaskCreatePinnedToCore(deadLoop, "Dead Loop", 1024 * 2, nullptr, 5, nullptr, 0);
}

void loop() {
  // 删除loopBack()任务，节约资源
  vTaskDelete(nullptr);
}
