#include <WiFi.h>

// ESP32所建立的TCP服务器参数
typedef struct {
  WiFiServer server;
  char wifiSSID[20];
  char wifiPassWord[20];
  int localPort;
  int maxClients;
  int currClients;
}ServerInfo;

ServerInfo myServer;
SemaphoreHandle_t mutexServer;
const TickType_t mutexTimeOut = 1000;

// 连接至服务器的客户端参数
typedef struct {
  WiFiClient client;
  int index;    // 表明是连接至服务器的第几个客户端
}ClientInfo;


/************************************************************************************
 * 函数: tcpInit()
 * 作用：初始化全局服务器参数，创建WiFi和TCP服务器
************************************************************************************/
void tcpInit() {
  // 初始化信息
  memcpy(myServer.wifiSSID, "ESP32_Multi_Server", strlen("ESP32_Multi_Server") + 1);
  memcpy(myServer.wifiPassWord, "SaligiaYZK", strlen("SaligiaYZK") + 1);
  myServer.localPort = 1688;
  myServer.maxClients = 2;
  myServer.currClients = 0;

  // 初始化互斥锁
  mutexServer = xSemaphoreCreateMutex();
  
  // 创建WIFI热点
  while (!WiFi.softAP(myServer.wifiSSID, myServer.wifiPassWord)) {
    Serial.println("Retrying...");
    vTaskDelay(500);
  }
  Serial.println("WiFi established!");

  // 建立TCP服务器
  myServer.server = WiFiServer(myServer.localPort);
  myServer.server.begin();
  Serial.println("TCP Server started!");
  Serial.print("Access point: ");Serial.println(myServer.wifiSSID);
  Serial.print("IP address: ");Serial.println(WiFi.softAPIP());
}


/************************************************************************************
 * 函数: tcpListen()
 * 作用：监听连接至服务器的客户端，并创建子线程处理各客户端请求
************************************************************************************/
void tcpListen(void *arg) {
  for (;;) {
    // 先判断是否超出最大TCP客户端要求，这里采取死等
    // 仅有本线程会增加客户端数量，所以为最小化锁的使用区间，判断完直接解锁，后面增加时再上锁
    if (xSemaphoreTake(mutexServer, portMAX_DELAY) == pdPASS) {
      if (myServer.currClients >= myServer.maxClients) {
        Serial.println("Too many connected clients!");
        xSemaphoreGive(mutexServer);  // 注意不要抱锁休眠
        vTaskDelay(2000);  // 客户端数目太多了，那就多休眠一会儿，释放CPU资源
        continue;
      }
      xSemaphoreGive(mutexServer);
    }

    ClientInfo *myClient = new ClientInfo();
    myClient->client = myServer.server.available();
    if (myClient->client) {
      xTaskCreate(tcpHandler, "TCP Handler", 1024 * 4, (void*)myClient, 2, nullptr);
      xSemaphoreTake(mutexServer, portMAX_DELAY);
      myServer.currClients++;
      xSemaphoreGive(mutexServer);
    }
    vTaskDelay(1000);
  }
}

/************************************************************************************
 * 函数: tcpHandler()
 * 作用：处理每一个线程的需求
************************************************************************************/
void tcpHandler(void *arg) {
  ClientInfo* clientInfo = (ClientInfo*) arg;
  Serial.println("Client connected!");
  clientInfo->client.println("Hello From ESP32 MultiThread Server!");
  while (clientInfo->client.connected()) {
    if (clientInfo->client.available()) {
      char c = clientInfo->client.read();
      Serial.print("Get info from client: ");Serial.println(c);
      clientInfo->client.print("Get info: ");clientInfo->client.println(c);
    }
    // 手动延时，否则会出现(1)tcpListen()无法执行(2)看门狗报错
    // 【TODO】这会导致发送连续指令时出现大面积延迟
    vTaskDelay(500);
  }
  clientInfo->client.stop();
  Serial.println("Client disconnected!");

  // 资源释放
  free(clientInfo);
  xSemaphoreTake(mutexServer, portMAX_DELAY);
  myServer.currClients--;
  xSemaphoreGive(mutexServer);
  vTaskDelete(nullptr);
}

void setup() {
  Serial.begin(115200);
  Serial.print("Chip model = ");Serial.println(ESP.getChipModel());
  Serial.print("Avaliable cores: ");Serial.println(ESP.getChipCores());
  Serial.println("Test for multiThread TCP Server...");
  Serial.println(" ");

  tcpInit();
  xTaskCreate(tcpListen, "Listen Thread", 1024 * 2, nullptr, 1, nullptr);

}

void loop() {
  vTaskDelete(nullptr);
}
