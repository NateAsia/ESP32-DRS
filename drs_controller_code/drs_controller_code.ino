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

/* --------------------- Definitions and static variables ------------------ */

  #include "constants.h"

  static const twai_filter_config_t   f_config =  {
                                                    .acceptance_code  = (DRS_INPUT_ID << 21),
                                                    .acceptance_mask  = ~(TWAI_STD_ID_MASK << 21),
                                                    .single_filter    = true
                                                  };
  static const twai_timing_config_t   t_config = TWAI_TIMING_CONFIG_1MBITS();     // CAN SPEED 
  static const twai_general_config_t  g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_GPIO_NUM, (gpio_num_t)RX_GPIO_NUM, TWAI_MODE_NO_ACK);

  static SemaphoreHandle_t rx_sem;
  static SemaphoreHandle_t tx_sem;


/* --------------------- DYNAMIC VARIABLES (I/O Status) ------------------ */

  // CAN
    twai_message_t message;         // CAN Message Struct

    twai_message_t transmit_msg = {
                                    .identifier = DRS_OUTPUT_ID, 
                                    .data_length_code = 1,
                                    .data = {0} 
                                  };

    bool drs_final_control  = false;

    bool drs_mode           = DRS_MODE_MANUAL;        // 0 = Manual, 1 = Low, 2 = Medium, 3 = High 
    bool drs_auto           = false;                  // if current environment variables indicate automatic DRS 
    bool drs_btn_manual     = false;                  // Normal DRS Button

    bool min_long_g         = false;
    bool threshold_met      = false;

    bool brakes_applied     = false;

    int lat_g               = 0;
    int lon_g               = 0;

    uint8_t brake_pressure  = 0;


/* --------------------------- Tasks and Functions -------------------------- */

void setup_io(){
  pinMode(LED_BUILTIN, OUTPUT);
}

bool check_auto_drs() {

  min_long_g = (lon_g > MIN_LONG_G);

  switch(drs_mode) {
    case DRS_MODE_MANUAL:
      return 0;

    case DRS_MODE_LO:
      threshold_met = lon_g > (lat_g * SLOPE_LOW_MODE);
      return (min_long_g && threshold_met);

    case DRS_MODE_HI:
      threshold_met = threshold_met = lon_g > (lat_g * SLOPE_HIGH_MODE);
      return (min_long_g && threshold_met);
    
    case DRS_MODE_OPEN:
      return 1;

    default:
      return 0;
  }

}

static void twai_transmit_task(void *arg){
    xSemaphoreTake(tx_sem, portMAX_DELAY);
    ESP_LOGI(BASE_TAG, "Transmit Thread Started");

    while(1){
      digitalWrite(LED_BUILTIN, HIGH);

        transmit_msg.data[0] = drs_final_control;

        ESP_ERROR_CHECK(twai_transmit(&transmit_msg , pdMS_TO_TICKS(5)));  

      digitalWrite(LED_BUILTIN, LOW);
      vTaskDelay(pdMS_TO_TICKS(10));    // Send updates @ 100 Hz
    }

    xSemaphoreGive(tx_sem);
    vTaskDelete(NULL);
}



static void twai_receive_task(void *arg){

    xSemaphoreTake(rx_sem, portMAX_DELAY);
    ESP_LOGI(BASE_TAG, "Receive Task Started");

    while(1){

        if (twai_receive(&message, pdMS_TO_TICKS(5)) == ESP_OK);
        else {
            printf("\nFailed to receive message\n");
            vTaskDelay(1);
            continue;
        }

        // Print received message
          if (message.extd) {
              printf("\n\nExtended Format\tID: 0x%.08x\t", message.identifier);
          } else {
              printf("\n\nStandard Format\tID: 0x%.03x\t", message.identifier);
          }
          if (!(message.rtr)) {
              printf("Data (%d) (hex): ", message.data_length_code);
              for (int i = 0; i < message.data_length_code; i++) {
                  printf("0x%.02x ", message.data[i]);
              }
          }

        // Process received message
        if(message.identifier == DRS_INPUT_ID){
          drs_mode        = message.data[0] & DRS_MODE_MASK;
          drs_btn_manual  = message.data[0] & DRS_BTN_MASK;
          lat_g           = (message.data[1] << 8) & message.data[2];
          lon_g           = (message.data[3] << 8) & message.data[4];
          brake_pressure  = message.data[5];


          brakes_applied = (brake_pressure > BRAKE_PRESSURE_LIMIT); // Brakes will always close DRS
          drs_auto = check_auto_drs();
          drs_final_control = (!brakes_applied) & (drs_btn_manual | drs_auto);

        }
        
        vTaskDelay(1); 
    }

    xSemaphoreGive(rx_sem);   // We should never reach this line :)
    vTaskDelete(NULL);

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

  printf("\n\t\tInitializing CAN\n\n");
  
  app_main();

}

void loop(){
  // do nothing in this loop, the FreeRTOS is managing task scheduling above
}
