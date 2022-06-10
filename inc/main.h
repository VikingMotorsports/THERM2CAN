#include "math.h"
#include "stm32f1xx.h"

//**DEFS**//
#define N_THERMISTORS 36
#define N_MUXS 6
#define THERM_PER_MUX 6
#define MODULE_NUMBER 4
#define LOW_ID 0x90
#define HIGH_ID 0xB3
#define CHECK_SUM 0x41

//** GLOBALS **//
CAN_HandleTypeDef hcan;
ADC_HandleTypeDef hadc;
CAN_TxHeaderTypeDef msg;
CAN_TxHeaderTypeDef err_msg;
CAN_TxHeaderTypeDef debug_msg;

//** PROTOTYPES **//
void blink(GPIO_TypeDef* port, uint16_t pin, uint8_t n);
