/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f1xx.h"
#include "math.h"

//TODO
//	move some stuff to main.h
//	make a config.c file
// 	get rid of claim_address & send_serial

//**DEFS**//
#define N_THERMISTORS 6
#define N_MUXS 3
#define THERM_PER_MUX 2
uint32_t r_0= 10000;
uint32_t adc_max= 4096;
uint32_t b= 3380;
uint32_t t_0= 298;

//** GLOBALS **//
CAN_HandleTypeDef hcan;
ADC_HandleTypeDef hadc;
CAN_TxHeaderTypeDef msg;

//** PROTOTYPES **//
void RCC_init();
void GPIO_init();
void CAN_init();
void ADC_init();
void blink(GPIO_TypeDef* port, uint16_t pin, uint8_t n);
void send_summary(uint8_t temperature);
void send_serial();
void claim_address();
void err();
void select_channel(uint8_t);


int main(void)
{
	HAL_Init();
	RCC_init();
	GPIO_init();
	ADC_init();
	CAN_init();
	uint8_t data[8];
	data[0] = 0; //TODO: update module_number for each module
	while(1)
	{
		uint8_t max= 0, min= 0, sum= 0;

		for(int i= 1;i <= N_THERMISTORS; ++i)
		{
			select_channel(i);
			HAL_ADC_Start(&hadc);
			HAL_ADC_PollForConversion(&hadc, 1000);
			double volts= HAL_ADC_GetValue(&hadc);
			HAL_ADC_Stop(&hadc);
			double r= (((double) adc_max * (double) r_0)/volts) - (double) r_0;
			double t= ((double) b/(log(r/(double) r_0)+ (double) b/(double) t_0)) - 273.15;
			//TODO: Check error conditions
			if(i == 1)
			{
				max = (uint8_t) t;
				min = (uint8_t) t;
			}

			if(t > max) max = (uint8_t) t;
			if(t < min) min = (uint8_t) t;
			sum += (uint8_t) t;
		}

		data[1] = min;
		data[2] = max;
		data[3] = sum/N_THERMISTORS;
		data[4] = 0x24;
		data[5] = 0x00;
		data[6] = 0x23;
		data[7]= 0x41; //TODO: verify this works
		for(int i= 0; i < 7; ++i)
		{
			data[7]+= data[i];
		}
		uint32_t mb;
		if (HAL_CAN_AddTxMessage(&hcan, &msg, data, &mb) == HAL_OK) {
			HAL_Delay(100);
		}
	}
}

//** INIT FUNCTIONS **//
void RCC_init()
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
//	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	/** Initializes the CPU, AHB and APB busses clocks
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		err();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
	{
		err();
	}

//	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
//	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
//	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
//	{
//		err();
//	}
}

void GPIO_init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

	/*Configure GPIO pin : PC13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin : PA5 */
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : PB6 */
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : PB7 */
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : PB8 */
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void ADC_init()
{
	hadc.Instance = ADC1;
	hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc.Init.ContinuousConvMode = DISABLE;
	hadc.Init.DiscontinuousConvMode = DISABLE;
	hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc.Init.NbrOfConversion = 1;
	if(HAL_ADC_Init(&hadc) != HAL_OK)
	{
		err();
	}
}
void CAN_init()
{
	hcan.Instance = CAN1;
	hcan.Init.Prescaler = 1;
	hcan.Init.Mode = CAN_MODE_NORMAL;
	hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
	hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
	hcan.Init.TimeTriggeredMode = DISABLE;
	hcan.Init.AutoBusOff = DISABLE;
	hcan.Init.AutoWakeUp = DISABLE;
	hcan.Init.AutoRetransmission = DISABLE;
	hcan.Init.ReceiveFifoLocked = DISABLE;
	hcan.Init.TransmitFifoPriority = DISABLE;
	if (HAL_CAN_Init(&hcan) != HAL_OK)
	{
		err();
	}
	/* USER CODE BEGIN CAN_Init 2 */
	CAN_FilterTypeDef sf;
	sf.FilterBank = 0;
	sf.FilterMode = CAN_FILTERMODE_IDMASK;
	sf.FilterScale = CAN_FILTERSCALE_16BIT;
	sf.FilterIdLow = 0x0000;
	sf.FilterIdHigh = 0x0000;
	sf.FilterMaskIdLow = 0x0000;
	sf.FilterMaskIdHigh = 0x0000;
	sf.FilterFIFOAssignment = CAN_RX_FIFO0;
	sf.SlaveStartFilterBank = 0;
	sf.FilterActivation = ENABLE;
	if (HAL_CAN_ConfigFilter(&hcan, &sf) != HAL_OK)
	{
		err();
	}

	if (HAL_CAN_Start(&hcan) != HAL_OK)
	{
		err();
	}

	msg.StdId = 0x80;
	msg.ExtId = 0x1839F380;
	msg.IDE = CAN_ID_EXT;
	msg.RTR = CAN_RTR_DATA;
	msg.DLC = 8;
	msg.TransmitGlobalTime = DISABLE;

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

//	TODO: My wiring is poorly planned so thermistors are on channel 2 and 1 of the MUXs
	if(mod == 1)
	{
		if(thermistor > 4)
		{
			sConfig.Channel = ADC_CHANNEL_2;
			if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
				err();
		}
		else if(thermistor > 2)
		{
			sConfig.Channel = ADC_CHANNEL_1;
			if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
				err();
		}
		else
		{
			sConfig.Channel = ADC_CHANNEL_0;
			if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
				err();
		}
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
	}


}
void send_summary(uint8_t temperature)
{
	CAN_TxHeaderTypeDef msg;

	msg.StdId = 0x80;
	msg.ExtId = 0x1839F380;
	msg.IDE = CAN_ID_EXT;
	msg.RTR = CAN_RTR_DATA;
	msg.DLC = 8;
	msg.TransmitGlobalTime = DISABLE;

	uint8_t txData[8];
	uint32_t mb;

	txData[0]= 0x00;
	txData[1]= temperature;
	txData[2]= temperature;
	txData[3]= temperature;
	txData[4]= 0x04;
	txData[5]= 0x00;
	txData[6]= 0x07;
	txData[7]= 0x41 + txData[6] + txData[5] + txData[4] + (3*temperature);


	if (HAL_CAN_AddTxMessage(&hcan, &msg, txData, &mb) == HAL_OK) {
//		blink(GPIOC, GPIO_PIN_13, 1);
	}

}
void send_serial()
{

}
void claim_address()
{

}
void err()
{
	blink(GPIOC, GPIO_PIN_13, 3);
}

//** MSP IMPLEMENTATION **//
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	RCC_PeriphCLKInitTypeDef  PeriphClkInit;
	__HAL_RCC_ADC1_CLK_ENABLE();
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	/*Configure GPIO pin : PA0 */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : PA1 */
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : PA2 */
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hcan->Instance==CAN1)
  {
    /* Peripheral clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }

}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan)
{
  if(hcan->Instance==CAN1)
  {
    __HAL_RCC_CAN1_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);
  }

}
