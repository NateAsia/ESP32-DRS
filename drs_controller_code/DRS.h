/* 
  ******************** DRS  ********************

  - Please use this for all DRS variables and functions

*/

/* --------------------------- Variables --------------------------- */

bool drs_final_control  = false;

bool drs_mode           = DRS_MODE_MANUAL;        // 0 = Manual, 1 = Low, 2 = Medium, 3 = High 
bool drs_auto           = false;                  // if current environment variables indicate automatic DRS 
bool drs_btn_manual     = false;                  // Normal DRS Button

bool min_long_g         = false;
bool threshold_met      = false;

bool brakes_applied     = false;

int lat_g               = 0;
int lon_g               = 0;


bool check_auto_drs() {

  Serial.print("MODE:");
  Serial.print(drs_mode);
  Serial.print("\tLON:");
  Serial.print(lon_g);
  Serial.print("\tLAT:");
  Serial.println(lat_g);

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