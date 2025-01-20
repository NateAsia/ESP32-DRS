#include "DRSController.h"

DRSController::DRSController()
    : drs_final_control(0)
    , drs_mode(DRS_MODE_MANUAL)
    , drs_auto(false)
    , drs_btn_manual(false)
    , min_long_g(false)
    , threshold_met(false)
    , brakes_applied(false)
    , lat_g(0)
    , lon_g(0)
{}

void DRSController::updateState(uint8_t mode, bool manual_btn, 
                              int16_t lateral_g, int16_t longitudinal_g, 
                              bool brakes) {
    drs_mode = mode;
    drs_btn_manual = manual_btn;
    lat_g = lateral_g;
    lon_g = longitudinal_g;
    brakes_applied = brakes;
    
    drs_auto = checkAutoDrs();
    calculateFinalControl();
}

bool DRSController::checkAutoDrs() {
    min_long_g = (lon_g > MIN_LONG_G);

    switch(drs_mode) {
        case DRS_MODE_MANUAL:
            return false;
        case DRS_MODE_LO:
            threshold_met = lon_g > (lat_g * SLOPE_LOW_MODE);
            return (min_long_g && threshold_met);
        case DRS_MODE_HI:
            threshold_met = lon_g > (lat_g * SLOPE_HIGH_MODE);
            return (min_long_g && threshold_met);
        case DRS_MODE_OPEN:
            return true;
        default:
            return false;
    }
}

void DRSController::calculateFinalControl() {
    drs_final_control = (!brakes_applied) & (drs_btn_manual | drs_auto);
}
