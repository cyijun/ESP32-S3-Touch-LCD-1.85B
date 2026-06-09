#include "I2C_Driver.h"
#include "debug_trace.h"

extern SemaphoreHandle_t wire_mutex;

void I2C_Init(void) {
  Wire.begin( I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setTimeOut(50);
}


bool I2C_Read(uint8_t Driver_addr, uint8_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
  if (!wire_mutex) {
    TRACE_DETAIL("I2C_Read: wire_mutex not initialized");
    return false;
  }
  if (xSemaphoreTake(wire_mutex, pdMS_TO_TICKS(50)) != pdTRUE) {
    TRACE_DETAIL("I2C_Read: mutex timeout");
    return false;
  }

  Wire.beginTransmission(Driver_addr);
  Wire.write(Reg_addr);
  if (Wire.endTransmission(true)) {
    TRACE_DETAIL("I2C_Read: endTransmission failed addr=0x%02x reg=0x%02x", Driver_addr, Reg_addr);
    xSemaphoreGive(wire_mutex);
    return false;
  }
  uint8_t received = Wire.requestFrom(Driver_addr, Length);
  if (received != Length) {
    TRACE_DETAIL("I2C_Read: requestFrom timeout expected=%lu got=%u", (unsigned long)Length, received);
    xSemaphoreGive(wire_mutex);
    return false;
  }
  for (uint32_t i = 0; i < Length; i++) {
    *Reg_data++ = Wire.read();
  }
  xSemaphoreGive(wire_mutex);
  return true;
}

bool I2C_Write(uint8_t Driver_addr, uint8_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
  if (!wire_mutex) {
    TRACE_DETAIL("I2C_Write: wire_mutex not initialized");
    return false;
  }
  if (xSemaphoreTake(wire_mutex, pdMS_TO_TICKS(50)) != pdTRUE) {
    TRACE_DETAIL("I2C_Write: mutex timeout");
    return false;
  }

  Wire.beginTransmission(Driver_addr);
  Wire.write(Reg_addr);
  for (uint32_t i = 0; i < Length; i++) {
    Wire.write(*Reg_data++);
  }
  if (Wire.endTransmission(true)) {
    TRACE_DETAIL("I2C_Write: endTransmission failed addr=0x%02x reg=0x%02x", Driver_addr, Reg_addr);
    xSemaphoreGive(wire_mutex);
    return false;
  }
  xSemaphoreGive(wire_mutex);
  return true;
}