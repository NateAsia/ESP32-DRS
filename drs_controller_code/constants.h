/* 
  ******************** CONSTANTS FILE ********************

  - Please use this for all variables that remain constant :)
  - Please label definitions with useful information on the right
  - Add new sections as needed

 */



/* --------------------- ESP-IDF Setup ------------------ */
  // LOG TAGS
    #define BASE_TAG    "DRS CONTROLLER"  



/* --------------------- FreeRTOS Setup ------------------ */
  #define TX_TASK_PRIO      9         // TX Task Priority 
  #define RX_TASK_PRIO      8         // RX Task Priority 
  #define INPUT_TASK_PRIO   7         // INPUT Task Priority
  #define OUTPUT_TASK_PRIO  6         // INPUT Task Priority


/* --------------------- TWAI / CAN Transceiver Setup ------------------ */
  #define TX_GPIO_NUM     8          // "TX2" on the ESP32-WROOM-32
  #define RX_GPIO_NUM     9          // "RX2" on the ESP32-WROOM-32

  #define DRS_INPUT_ID    0x653          // From the C185 or steering wheel
  #define DRS_OUTPUT_ID   0x654          // To ECU (or back to the C185 first)


/* --------------------- DRS-C IO ------------------ */
  // Buttons
    #define BUTTON_1_PIN       // Location:? Color:? Purpose: 
    #define BUTTON_2_PIN       // Location:? Color:? Purpose: 
    #define LED_BUILTIN   2

/* --------------------- Logic Code ------------------ */
  // Modes
    #define DRS_MODE_MANUAL       0
    #define DRS_MODE_LO           1
    #define DRS_MODE_HI           2
    #define DRS_MODE_OPEN         3
  
  // Brakes
    #define BRAKE_PRESSURE_LIMIT  50   // PSI

  // ACCEL HI
    #define SLOPE_HIGH_MODE           0.5 // G per G

  // ACCEL LOW
    #define SLOPE_LOW_MODE            1.0 // G per G

  // FLAT ZONE
    #define MIN_LONG_G                0.1 // G 

  // BIT MASKS
    #define DRS_MODE_MASK 0b00000011       
    #define DRS_BTN_MASK  0b00000100




    