#include "math.h"
#include "stm32f1xx.h"

//**DEFS**//
#define N_THERMISTORS 36
#define N_MUXS 6
#define THERM_PER_MUX 6
#define MODULE_NUMBER 0

//** GLOBALS **//
CAN_HandleTypeDef hcan;
ADC_HandleTypeDef hadc;
CAN_TxHeaderTypeDef msg;
CAN_TxHeaderTypeDef err_msg;
CAN_TxHeaderTypeDef debug_msg;

//** PROTOTYPES **//
void blink(GPIO_TypeDef* port, uint16_t pin, uint8_t n);
