#include "CANHandler.h"
#include "esp_log.h"

const twai_filter_config_t CANHandler::f_config = {
    .acceptance_code = (DRS_INPUT_ID << 21),
    .acceptance_mask = ~(TWAI_STD_ID_MASK << 21),
    .single_filter = true
};

const twai_timing_config_t CANHandler::t_config = TWAI_TIMING_CONFIG_1MBITS();
const twai_general_config_t CANHandler::g_config = TWAI_GENERAL_CONFIG_DEFAULT(
    (gpio_num_t)TX_GPIO_NUM, 
    (gpio_num_t)RX_GPIO_NUM, 
    TWAI_MODE_NO_ACK
);

CANHandler::CANHandler() 
    : message_callback(nullptr)
    , state_callback(nullptr)
    , rx_sem(nullptr)
    , tx_sem(nullptr)
    , rx_task_handle(nullptr)
    , tx_task_handle(nullptr)
    , running(false) {
    
    rx_sem = xSemaphoreCreateBinary();
    tx_sem = xSemaphoreCreateBinary();
    
    transmit_msg.identifier = DRS_OUTPUT_ID;
    transmit_msg.data_length_code = CAN_MSG_LENGTH;
    transmit_msg.data[0] = ONLINE_BIT_VALUE;
    for(int i = 1; i < CAN_MSG_LENGTH; i++) {
        transmit_msg.data[i] = 0;
    }
}

CANHandler::~CANHandler() {
    stopTasks();
    if (rx_sem) vSemaphoreDelete(rx_sem);
    if (tx_sem) vSemaphoreDelete(tx_sem);
}

bool CANHandler::initialize() {
    if (!message_callback || !state_callback) {
        ESP_LOGE(BASE_TAG, "Callbacks not set");
        return false;
    }

    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        ESP_LOGE(BASE_TAG, "Failed to install TWAI driver");
        return false;
    }
    
    if (twai_start() != ESP_OK) {
        ESP_LOGE(BASE_TAG, "Failed to start TWAI driver");
        return false;
    }
    
    xSemaphoreGive(rx_sem);
    xSemaphoreGive(tx_sem);
    running = true;
    return true;
}

void CANHandler::startTasks() {
    xTaskCreate(twai_receive_task, "TWAI_rx", STACK_SIZE, this, RX_TASK_PRIO, &rx_task_handle);
    xTaskCreate(twai_transmit_task, "TWAI_tx", STACK_SIZE, this, TX_TASK_PRIO, &tx_task_handle);
}

void CANHandler::stopTasks() {
    running = false;
    if (rx_task_handle) vTaskDelete(rx_task_handle);
    if (tx_task_handle) vTaskDelete(tx_task_handle);
    twai_stop();
    twai_driver_uninstall();
}

void CANHandler::processReceivedMessage(const twai_message_t& message) {
    if(message.identifier == DRS_INPUT_ID && message_callback) {
        uint8_t mode = message.data[0] & DRS_MODE_MASK;
        bool manual_btn = message.data[0] & DRS_BTN_MASK;
        int16_t lat_g = (message.data[1] << 8) | message.data[2];
        int16_t lon_g = (message.data[3] << 8) | message.data[4];
        bool brakes = message.data[5];
        
        message_callback(mode, manual_btn, lat_g, lon_g, brakes);
    }
}

void CANHandler::prepareTransmitMessage() {
    if (state_callback) {
        transmit_msg.data[1] = state_callback();
    }
}

void CANHandler::twai_receive_task(void* arg) {
    CANHandler* handler = static_cast<CANHandler*>(arg);
    twai_message_t message;
    
    xSemaphoreTake(handler->rx_sem, portMAX_DELAY);
    ESP_LOGI(BASE_TAG, "Receive Task Started");

    while(handler->running) {
        if (twai_receive(&message, pdMS_TO_TICKS(RX_TIMEOUT_MS)) == ESP_OK) {
            handler->processReceivedMessage(message);
        }
        vTaskDelay(1);
    }
    
    vTaskDelete(NULL);
}

void CANHandler::twai_transmit_task(void* arg) {
    CANHandler* handler = static_cast<CANHandler*>(arg);
    
    xSemaphoreTake(handler->tx_sem, portMAX_DELAY);
    ESP_LOGI(BASE_TAG, "Transmit Task Started");

    while(handler->running) {
        digitalWrite(LED_BUILTIN, HIGH);
        
        handler->prepareTransmitMessage();
        ESP_ERROR_CHECK(twai_transmit(&handler->transmit_msg, pdMS_TO_TICKS(TX_TIMEOUT_MS)));
        
        digitalWrite(LED_BUILTIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(TX_INTERVAL_MS));
    }
    
    vTaskDelete(NULL);
}
