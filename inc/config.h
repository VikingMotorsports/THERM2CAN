#include "main.h"

//** PROTOTYPES **//
void RCC_init();
void GPIO_init();
void CAN_init();
void ADC_init();
void send_serial();
void claim_address();
void err();
void select_channel(uint8_t);
