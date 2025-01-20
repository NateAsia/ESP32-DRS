#pragma once

/* --------------------- ESP-IDF Setup ------------------ */
#define BASE_TAG "DRS_CONTROLLER"

/* --------------------- FreeRTOS Setup ------------------ */
#define TX_TASK_PRIO      9
#define RX_TASK_PRIO      8
#define STACK_SIZE        4096
#define TX_INTERVAL_MS    10      // 100Hz transmission rate
#define RX_TIMEOUT_MS     5
#define TX_TIMEOUT_MS     5

/* --------------------- TWAI / CAN Setup ------------------ */
#define TX_GPIO_NUM     8          // TX2 on ESP32-WROOM-32
#define RX_GPIO_NUM     9          // RX2 on ESP32-WROOM-32
#define DRS_INPUT_ID    0x653      // From C185 or steering wheel
#define DRS_OUTPUT_ID   0x654      // To C185
#define CAN_SPEED      1000000     // 1Mbps

/* --------------------- IO Pins ------------------ */
#define LED_BUILTIN    2

/* --------------------- DRS Modes ------------------ */
#define DRS_MODE_MANUAL    0
#define DRS_MODE_LO        1
#define DRS_MODE_HI        2
#define DRS_MODE_OPEN      3

/* --------------------- DRS Parameters ------------------ */
#define SLOPE_HIGH_MODE    0.5     // G per G
#define SLOPE_LOW_MODE     1.0     // G per G
#define MIN_LONG_G         0.1     // G

/* --------------------- CAN Message Masks ------------------ */
#define DRS_MODE_MASK      0b00000011
#define DRS_BTN_MASK       0b00000100

/* --------------------- CAN Message Definitions ------------------ */
#define CAN_MSG_LENGTH     8
#define ONLINE_BIT_VALUE   1

    
