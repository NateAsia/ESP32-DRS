#pragma once
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "constants.h"

class CANHandler {
public:
    using MessageCallback = void(*)(uint8_t, bool, int16_t, int16_t, bool);
    using StateCallback = uint8_t(*)();

    CANHandler();
    ~CANHandler();
    
    void setMessageCallback(MessageCallback cb) { message_callback = cb; }
    void setStateCallback(StateCallback cb) { state_callback = cb; }
    
    bool initialize();
    void startTasks();
    void stopTasks();
    
private:
    static void twai_receive_task(void* arg);
    static void twai_transmit_task(void* arg);
    void processReceivedMessage(const twai_message_t& message);
    void prepareTransmitMessage();
    
    MessageCallback message_callback;
    StateCallback state_callback;
    SemaphoreHandle_t rx_sem;
    SemaphoreHandle_t tx_sem;
    twai_message_t transmit_msg;
    TaskHandle_t rx_task_handle;
    TaskHandle_t tx_task_handle;
    bool running;
    
    static const twai_filter_config_t f_config;
    static const twai_timing_config_t t_config;
    static const twai_general_config_t g_config;
};
