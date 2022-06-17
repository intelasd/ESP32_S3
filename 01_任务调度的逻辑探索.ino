/*
*   对于任务优先级进行探索
*/

void task1(void *arg) {
  for (;;) {
    Serial.print("Task 1 - Priority ");
    Serial.println(uxTaskPriorityGet(NULL));
  }
}

void task2(void *arg) {
  for (;;) {
    Serial.print("Task 2 - Priority ");
    Serial.println(uxTaskPriorityGet(NULL));

    // vTaskDelay() 可以将资源释放给优先级更低的任务 
    // vTaskDelay(10);
    // taskYIELD() 只能将资源释放给优先级更高或相同的任务
    // taskYIELD();
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  // xTaskCreate(task1, "task1", 1024 * 2, NULL, 2, NULL);
  // xTaskCreate(task2, "task2", 1024 * 2, NULL, 2, NULL);

  // 同等级的任务可以同时轮流运行
  // xTaskCreatePinnedToCore(task1, "task1", 1024 * 2, NULL, 2, NULL, 1);
  // xTaskCreatePinnedToCore(task2, "task2", 1024 * 2, NULL, 2, NULL, 1);

  // 后来的高优先级可以进行抢占
  // xTaskCreatePinnedToCore(task1, "task1", 1024 * 2, NULL, 2, NULL, 1);
  // vTaskDelay(2000);
  // xTaskCreatePinnedToCore(task2, "task2", 1024 * 2, NULL, 3, NULL, 1);

  // 如果两个任务在不同核心上，那么高优先级和低优先级可以完全并行，互不影响
  // xTaskCreatePinnedToCore(task1, "task1", 1024 * 2, NULL, 1, NULL, 0);
  // xTaskCreatePinnedToCore(task2, "task2", 1024 * 2, NULL, 2, NULL, 1);

  // 如果不指定核心，那么高优先级和低优先级可以完全并行，互不影响
  // xTaskCreate(task1, "task1", 1024 * 2, NULL, 1, NULL);
  // xTaskCreate(task2, "task2", 1024 * 2, NULL, 2, NULL);
  
  // 验证Loop()的优先级为【1】
  // 这里loop无法启动，因为只有执行完setup，才会执行loop，这时高等级任务已经启动，loop被抢占
  // freeRTOS源码中 创建了LoopTask(优先级1)，在其中先执行setup，才会执行loop
  // xTaskCreatePinnedToCore(task1, "task1", 1024 * 2, NULL, 1, NULL, 1);
  // vTaskDelay(2000);
  // xTaskCreatePinnedToCore(task2, "task2", 1024 * 2, NULL, 2, NULL, 1);

  // 这里核心0上task2任务抢占task1，核心1上loop任务不受影响
  xTaskCreatePinnedToCore(task1, "task1", 1024 * 2, NULL, 1, NULL, 0);
  vTaskDelay(2000);
  xTaskCreatePinnedToCore(task2, "task2", 1024 * 2, NULL, 2, NULL, 0);
}

void loop() {
  Serial.print("Task Loop - Priority ");
  Serial.println(uxTaskPriorityGet(NULL));
}
