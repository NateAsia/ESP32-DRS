/* --------------------- Standard / ESP Specific Imports ------------------ */ // FYI use (command + click) on a import or variable to goto its definition for more info 
  #include <stdio.h>                // Yummy OS Stuff
  #include <stdlib.h>               // Alot of helpful yummy libs
  #include "freertos/FreeRTOS.h"    // Task Management
  #include "freertos/task.h"        // Task Management
  #include "freertos/queue.h"       // Task Management 
  #include "freertos/semphr.h"      // Task Management
  #include "esp_err.h"              // ESP32 Errors     --  used for debugging
  #include "esp_log.h"              // ESP32 Logging    --  used for debugging
  #include "driver/twai.h"          // API HERE --> [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/twai.html]

/* --------------------- Custom Imports ------------------ */
  #include "constants.h"
  #include "DRS.h"
  #include "FSAE_CAN.h"
  #include "DRSController.h"
  #include "CANHandler.h"
/* --------------------------- Tasks and Functions -------------------------- */

DRSController drs_controller;
CANHandler* can_handler = nullptr;

void setup_io() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

void onCANMessage(uint8_t mode, bool manual_btn, int16_t lat_g, int16_t lon_g, bool brakes) {
    drs_controller.updateState(mode, manual_btn, lat_g, lon_g, brakes);
}

uint8_t getControlState() {
    return drs_controller.getFinalControl();
}

void setup() {
    ESP_LOGI(BASE_TAG, "ESP-STARTING");
    setup_io();
    
    can_handler = new CANHandler();
    can_handler->setMessageCallback(onCANMessage);
    can_handler->setStateCallback(getControlState);
    
    if (can_handler->initialize()) {
        can_handler->startTasks();
        ESP_LOGI(BASE_TAG, "CAN handler initialized successfully");
    } else {
        ESP_LOGE(BASE_TAG, "Failed to initialize CAN handler");
    }
}

void loop() {
    // FreeRTOS handles task scheduling
    vTaskDelay(pdMS_TO_TICKS(1000));
}
