/* 
  ******************** CAN  ********************

  - Please use this for all TWAI/CAN variables and functions

 */


/* --------------------------- Variables --------------------------- */

static const twai_filter_config_t   f_config =  {
                                                  .acceptance_code  = (DRS_INPUT_ID << 21),
                                                  .acceptance_mask  = ~(TWAI_STD_ID_MASK << 21),
                                                  .single_filter    = true
                                                };
static const twai_timing_config_t   t_config = TWAI_TIMING_CONFIG_1MBITS();     // CAN SPEED 
static const twai_general_config_t  g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_GPIO_NUM, (gpio_num_t)RX_GPIO_NUM, TWAI_MODE_NO_ACK);

twai_message_t message;

twai_message_t transmit_msg = {
                                .identifier = DRS_OUTPUT_ID, 
                                .data_length_code = 8,
                                .data = {
                                  1,          // ONLINE BIT
                                  0,          // DRS CONTROL BIT
                                  0,0,0,0,0,0 // Unused

                                } 
                              };

static SemaphoreHandle_t rx_sem;
static SemaphoreHandle_t tx_sem;




/* --------------------------- Tasks --------------------------- */

// CAN/TWAI Transmit Task
static void twai_transmit_task(void *arg){
    xSemaphoreTake(tx_sem, portMAX_DELAY);
    ESP_LOGI(BASE_TAG, "Transmit Thread Started");

    while(1){
      digitalWrite(LED_BUILTIN, HIGH);

      transmit_msg.data[1] = drs_final_control;

      ESP_ERROR_CHECK(twai_transmit(&transmit_msg , pdMS_TO_TICKS(5)));  

      digitalWrite(LED_BUILTIN, LOW);
      vTaskDelay(pdMS_TO_TICKS(10));    // Send updates @ 100 Hz
    }

    xSemaphoreGive(tx_sem);
    vTaskDelete(NULL);
}

// CAN/TWAI Recieve Task
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
          brakes_applied  = message.data[5];

          drs_auto = check_auto_drs();
          drs_final_control = (!brakes_applied) & (drs_btn_manual | drs_auto);


        }
        
        vTaskDelay(1); 
    }

    xSemaphoreGive(rx_sem);   // We should never reach this line :)
    vTaskDelete(NULL);

}