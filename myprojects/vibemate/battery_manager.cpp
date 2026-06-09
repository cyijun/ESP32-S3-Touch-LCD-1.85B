#include "battery_manager.h"
#include "I2C_Driver.h"
#include "debug_trace.h"
#include <BQ27220.h>

extern SemaphoreHandle_t wire_mutex;

static BQ27220 s_bq27220;
static bool s_available = false;

bool battery_init(void)
{
    TRACE_MAIN("battery_init start");
    if (!s_bq27220.begin(Wire, 0x55, I2C_SDA_PIN, I2C_SCL_PIN, 400000)) {
        TRACE_MAIN("BQ27220 not found");
        s_available = false;
        return false;
    }
    TRACE_MAIN("BQ27220 ready");
    s_available = true;
    return true;
}

BatteryData battery_read(void)
{
    BatteryData data = {0, 0, 0, 0.0f, false};

    if (!s_available) {
        return data;
    }

    if (wire_mutex != NULL &&
        xSemaphoreTake(wire_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        data.soc = s_bq27220.readStateOfChargePercent();
        data.voltage_mv = s_bq27220.readVoltageMillivolts();
        data.current_ma = s_bq27220.readCurrentMilliamps();
        data.temp_c = s_bq27220.readTemperatureCelsius();
        data.valid = true;
        xSemaphoreGive(wire_mutex);
    }

    return data;
}

bool battery_is_available(void)
{
    return s_available;
}
