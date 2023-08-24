/*
 * myDWM3000.c
 *
 *  Created on: 9 lis 2022
 *      Author: Mati
 */

#include "main.h"
#include "myDWM3000.h"
#include <string.h>
#include <stdio.h>

SPI_HandleTypeDef *DWM_spi;

UART_HandleTypeDef *DWM_uart;

uint8_t *rx_buffer;
static uint8_t SendBuffer[2];

void Init(SPI_HandleTypeDef *hspi, uint8_t *rxbuffer)
{
	DWM_spi = hspi;
	rx_buffer = rxbuffer;
}

static void DWM3000_SendToDWM(uint8_t *Byte, uint32_t Length)
{
	HAL_SPI_Transmit(DWM_spi, Byte, Length, DWM_SPI_TIMEOUT);
}

static void DWM3000_ReceiveFromDWM(uint8_t *ReceiveByte, uint32_t Length)
{
	HAL_SPI_Receive(DWM_spi, ReceiveByte, Length, DWM_SPI_TIMEOUT);
}

void UART_Log(char *Data)
{
	HAL_UART_Transmit(DWM_uart, (uint8_t* ) Data, strlen(Data), 30);
}


static void DWM3000_Create16bitCommand(uint16_t *Command, uint32_t BaseReg, uint32_t ReadOrWrite, uint32_t SubReg)
{
	uint16_t sub_add_temp;

	*Command = BaseReg << 9;
	*Command = *Command | ReadOrWrite;
	*Command = *Command | DWM_16BIT_MASK;
	sub_add_temp = SubReg << 2;
	*Command = *Command | sub_add_temp;
}

static void DWM3000_GetRXData16(uint16_t *Command, uint8_t SizeofData)
{
	SendBuffer[0] = (*Command >> 8);
	SendBuffer[1] = (*Command & 0xFF);
	DWM_CS_LOW;
	//HAL_SPI_Transmit(DWM_spi, SendBuffer, 2, DWM_SPI_TIMEOUT);
	DWM3000_SendToDWM(SendBuffer, 2);
	//HAL_SPI_TransmitReceive(DWM_spi, SendBuffer, ReceiveBuffer, SizeofData + 2, DWM_SPI_TIMEOUT);
	DWM3000_ReceiveFromDWM(rx_buffer, SizeofData);
	//HAL_SPI_Receive(DWM_spi, ReceiveBuffer, SizeofData, DWM_SPI_TIMEOUT);
	DWM_CS_HIGH;
}

void DWM3000_Read_RX_Data(void)
{
	uint16_t Command;
	//char RxData[16];
	DWM3000_Create16bitCommand(&Command,DWM_RXDATA_FILE_ID, DWM_READ_MASK, DWM_RX_BUFFER_0);
	DWM3000_GetRXData16(&Command, 24);

//	for (uint8_t i = 0; i < FRAME_LENGTH - FCS_LEN; i++)
//	{
//		RxData[i] = DWM_RX_data[i];
//	}
}

