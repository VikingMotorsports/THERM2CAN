#include "math.h"
#include "stm32f1xx.h"

//**DEFS**//
#define N_THERMISTORS 6
#define N_MUXS 3
#define THERM_PER_MUX 2

//** GLOBALS **//
CAN_HandleTypeDef hcan;
ADC_HandleTypeDef hadc;
CAN_TxHeaderTypeDef msg;

//** PROTOTYPES **//
void blink(GPIO_TypeDef* port, uint16_t pin, uint8_t n);
void err();
