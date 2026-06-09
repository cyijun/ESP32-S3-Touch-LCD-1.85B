#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>

struct BatteryData {
    int soc;       // State of charge (%)
    int voltage_mv;
    int current_ma;
    float temp_c;
    bool valid;    // false if read failed
};

// Initialize the battery manager (call in setup, replaces BQ27220 init in vibemate.ino)
bool battery_init(void);

// Read battery data (internally acquires wire_mutex)
BatteryData battery_read(void);

// Check if BQ27220 is available
bool battery_is_available(void);

#endif
