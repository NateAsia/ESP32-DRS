#pragma once
#include <stdint.h>
#include "constants.h"

class DRSController {
public:
    DRSController();
    
    void updateState(uint8_t mode, bool manual_btn, int16_t lateral_g, 
                    int16_t longitudinal_g, bool brakes);
    uint8_t getFinalControl() const { return drs_final_control; }
    
private:
    bool checkAutoDrs();
    void calculateFinalControl();

    // State variables
    uint8_t drs_final_control;
    uint8_t drs_mode;
    bool drs_auto;
    bool drs_btn_manual;
    bool min_long_g;
    bool threshold_met;
    bool brakes_applied;
    int16_t lat_g;
    int16_t lon_g;
};
