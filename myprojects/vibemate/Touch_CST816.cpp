#include "Touch_CST816.h"
#include "I2C_Driver.h"

struct CST816_Touch touch_data = {0};
uint8_t Touch_interrupts=0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I2C — delegated to I2C_Driver (which already protects the bus with wire_mutex)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool I2C_Read_Touch(uint16_t Driver_addr, uint8_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
  return I2C_Read((uint8_t)Driver_addr, Reg_addr, Reg_data, Length);
}

bool I2C_Write_Touch(uint8_t Driver_addr, uint8_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
  return I2C_Write(Driver_addr, Reg_addr, Reg_data, Length);
}
/*!
    @brief  handle interrupts
*/
void ARDUINO_ISR_ATTR Touch_CST816_ISR(void) {
  Touch_interrupts = true;
}

uint8_t Touch_Init(void) {
  pinMode(CST816_RST_PIN, OUTPUT);       // 将 GPIO5 设置为输出模式
  CST816_Touch_Reset();
  uint16_t Verification = CST816_Read_cfg();
  CST816_AutoSleep(true);
   
  pinMode(CST816_INT_PIN, INPUT_PULLUP);
  attachInterrupt(CST816_INT_PIN, Touch_CST816_ISR, FALLING); 

  return true;
}
/* Reset controller */
uint8_t CST816_Touch_Reset(void)
{
  digitalWrite(CST816_RST_PIN, LOW); 
  vTaskDelay(pdMS_TO_TICKS(10));
  digitalWrite(CST816_RST_PIN, HIGH);
  vTaskDelay(pdMS_TO_TICKS(50));
  return true;
}
uint16_t CST816_Read_cfg(void) {

  uint8_t buf[3];
  I2C_Read_Touch(CST816_ADDR, CST816_REG_Version,buf, 1);
  printf("TouchPad_Version:0x%02x\r\n", buf[0]);
  I2C_Read_Touch(CST816_ADDR, CST816_REG_ChipID, buf, 3);
  printf("ChipID:0x%02x   ProjID:0x%02x   FwVersion:0x%02x \r\n",buf[0], buf[1], buf[2]);

  return true;
}
/*!
    @brief  Fall asleep automatically
*/
void CST816_AutoSleep(bool Sleep_State) {
  CST816_Touch_Reset();
  uint8_t Sleep_State_Set = (uint8_t)(!Sleep_State);
  Sleep_State_Set = 10;
  I2C_Write_Touch(CST816_ADDR, CST816_REG_DisAutoSleep, &Sleep_State_Set, 1);
}

// reads sensor and touches
// updates Touch Points
uint8_t Touch_Read_Data(void) {
  uint8_t buf[6] = {0};
  if (!I2C_Read_Touch(CST816_ADDR, CST816_REG_GestureID, buf, 6)) {
    /* I2C failed — report "no touch" so LVGL does not think finger is held.
       Keeping stale coordinates is safer than reporting garbage. */
    touch_data.points = 0;
    return false;
  }
  /* touched gesture */
  if (buf[0] != 0x00)
    touch_data.gesture = (GESTURE)buf[0];
  /* Parse before touching global state */
  uint8_t points = (uint8_t)buf[1];
  if (points > CST816_LCD_TOUCH_MAX_POINTS)
      points = CST816_LCD_TOUCH_MAX_POINTS;

  uint16_t x = 0, y = 0;
  bool coords_invalid = false;
  if (points > 0) {
      x = ((buf[2] & 0x0F) << 8) + buf[3];
      y = ((buf[4] & 0x0F) << 8) + buf[5];
      if (x > 400 || y > 400) {
          points = 0;
          coords_invalid = true;
      }
  }

  noInterrupts();
  touch_data.points = points;
  if (points > 0) {
      touch_data.x = x;
      touch_data.y = y;
  }
  interrupts();

  if (coords_invalid) {
      printf("[DIAG] Touch_Read_Data invalid coords: x=%u y=%u\r\n", x, y);
  }
  return true;
}
void example_touchpad_read(void){
  Touch_Read_Data();
  if (touch_data.gesture != NONE ||  touch_data.points != 0x00) {
      printf("Touch : X=%u Y=%u points=%d\r\n",  touch_data.x , touch_data.y,touch_data.points);
  } else {
      // data->state = LV_INDEV_STATE_REL;
  }
}
void Touch_Loop(void){
  if(Touch_interrupts){
    Touch_interrupts = false;
    example_touchpad_read();
  }
}


/*!
    @brief  get the gesture event name
*/
String Touch_GestureName(void) {
  switch (touch_data.gesture) {
    case NONE:
      return "NONE";
      break;
    case SWIPE_DOWN:
      return "SWIPE DOWN";
      break;
    case SWIPE_UP:
      return "SWIPE UP";
      break;
    case SWIPE_LEFT:
      return "SWIPE LEFT";
      break;
    case SWIPE_RIGHT:
      return "SWIPE RIGHT";
      break;
    case SINGLE_CLICK:
      return "SINGLE CLICK";
      break;
    case DOUBLE_CLICK:
      return "DOUBLE CLICK";
      break;
    case LONG_PRESS:
      return "LONG PRESS";
      break;
    default:
      return "UNKNOWN";
      break;
  }
}