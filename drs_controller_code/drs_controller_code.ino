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
/* --------------------------- Tasks and Functions -------------------------- */

void setup_io(){
  pinMode(LED_BUILTIN, OUTPUT);
}

void app_main(void){

    // Create Semaphore(s)
    rx_sem = xSemaphoreCreateBinary();  
    tx_sem = xSemaphoreCreateBinary();  

    // CREATE THREADS (TASKS)
    xTaskCreatePinnedToCore(twai_receive_task,    "TWAI_rx",        4096, NULL, RX_TASK_PRIO,     NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(twai_transmit_task,   "TWAI_tx",        4096, NULL, TX_TASK_PRIO,     NULL, tskNO_AFFINITY);

    // Install and start TWAI driver            -   This will force the ESP32 to restart if there is a CAN error - good
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(BASE_TAG, "CAN Driver installed");
    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(BASE_TAG, "CAN Driver started");

    xSemaphoreGive(rx_sem);                     // Allow Start of RX task   
    xSemaphoreGive(tx_sem);                     // Allow Start of TX task
    vTaskDelay(pdMS_TO_TICKS(100));             
    
    xSemaphoreTake(rx_sem, portMAX_DELAY);      // Wait for RX Task to complete (never ;) )
    xSemaphoreTake(tx_sem, portMAX_DELAY);      // Wait for TX task to complete (never ;) )
    printf("\nEXITING\n");                      // (we will NEVER reach this line)

    //Stop and uninstall TWAI driver            // Just incase tho
    ESP_ERROR_CHECK(twai_stop());
    ESP_ERROR_CHECK(twai_driver_uninstall());
    ESP_LOGI(BASE_TAG, "CAN Driver Uninstalled");

    //Cleanup
    vSemaphoreDelete(rx_sem);
    vSemaphoreDelete(tx_sem);
}

/* --------------------------- Arduino IDE Compliance --------------------------- */
void setup(){
  
  ESP_LOGI(BASE_TAG, "ESP-STARTING");
  setup_io();
  app_main();

}

void loop(){
  // do nothing in this loop, the FreeRTOS is managing task scheduling above
}
