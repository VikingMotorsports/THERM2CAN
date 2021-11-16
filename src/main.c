#include "main.h"
#include "config.h"

uint32_t r2= 21500;
uint32_t r_0= 10000;
uint32_t adc_max= 4096;
uint32_t b= 3380;
uint32_t t_0= 298;

void RCC_init();
void GPIO_init();
void CAN_init();
void ADC_init();
void err();

int main(void)
{
	HAL_Init();
	RCC_init();
	GPIO_init();
	ADC_init();
	CAN_init();

	uint32_t mb;
	uint8_t data[8];
	data[0] = MODULE_NUMBER;
	while(1)
	{
		uint8_t max= 0, min= 0, errors= 0;
		uint32_t sum= 0;

		for(int i= 1;i <= N_THERMISTORS; ++i)
		{
			if( i == 25 || i == 26 ){
				sum += 25;
				continue;
			}

			select_channel(i);

			HAL_ADC_Start(&hadc);
			HAL_ADC_PollForConversion(&hadc, 1000);
			double adc_val= HAL_ADC_GetValue(&hadc);
			HAL_ADC_Stop(&hadc);

			if(adc_val > 4090)
			{
				++errors;
				data[4] = 0x80;
			}
			else
			{

				double volts = (adc_val/adc_max) * 3.3;
				double r= (volts * (double) r2)/((double) 5.0 - volts);
				double t= (503620.0 / (1690.0 + (149 * log(r/r_0)))) - 273.15;

				if(t > 255) //highest temperature value possible in CAN message is 255, so set error bit if this overflows
				{
					data[4] = 0x80;
				}

				if(i == 1)
				{
					max = (uint8_t) t;
					min = (uint8_t) t;
				}
				else if(t > max)
				{
					max = (uint8_t) t;
				}
				else if(t < min) min = (uint8_t) t;

				sum += t;
			}
		}

		data[1] = min;
		data[2] = max;
		data[3] = (uint8_t) (sum/N_THERMISTORS);
		data[4] += (0x24 - errors);
		data[5] = 0x0;
		data[6] = 0x23;
		data[7]= 0x41;
		for(int i= 0; i < 7; ++i)
		{
			data[7]+= data[i];
		}

		if (HAL_CAN_AddTxMessage(&hcan, &msg, data, &mb) == HAL_OK) {
			HAL_Delay(100);
		}
		else {
			err();
		}
		data[4] = 0;
	}
}

//** USER FUNCTIONS **//
void blink(GPIO_TypeDef* port, uint16_t pin, uint8_t n){
	for(int i= 0; i < n; ++i){
		HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
		HAL_Delay(500);
	}
}

//Selects the ADC channel and turns on the necessary SELECT pins for the MUXs
void select_channel(uint8_t thermistor)
{
	uint8_t mod= thermistor % THERM_PER_MUX;

	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;

	if(thermistor > 30) {
		sConfig.Channel = ADC_CHANNEL_6;
	}
	else if(thermistor > 24) {
		sConfig.Channel = ADC_CHANNEL_5;
	}
	else if(thermistor > 18) {
		sConfig.Channel = ADC_CHANNEL_4;
	}
	else if(thermistor > 12) {
		sConfig.Channel = ADC_CHANNEL_3;
	}
	else if(thermistor > 6) {
		sConfig.Channel = ADC_CHANNEL_2;
	}
	else if(thermistor > 0) {
		sConfig.Channel = ADC_CHANNEL_1;
	}

	if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
		err();

	if(mod & 1)
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

	if(mod & 2)
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);

	if(mod & 4)
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
}
void err()
{
	blink(GPIOC, GPIO_PIN_13, 100);
}
