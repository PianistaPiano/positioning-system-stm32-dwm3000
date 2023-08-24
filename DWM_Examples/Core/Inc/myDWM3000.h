/*
 * myDWM3000.h
 *
 *  Created on: 9 lis 2022
 *      Author: Mati
 */

#ifndef INC_MYDWM3000_H_
#define INC_MYDWM3000_H_


#define DWM_SPI_TIMEOUT 1000

#define DWM_CS_LOW HAL_GPIO_WritePin(DW_NSS_GPIO_Port, DW_NSS_Pin, GPIO_PIN_RESET)
#define DWM_CS_HIGH HAL_GPIO_WritePin(DW_NSS_GPIO_Port, DW_NSS_Pin, GPIO_PIN_SET)

#define DWM_READ_MASK 	0x0000 // for 16bit Command
#define DWM_WRITE_MASK 	0x8000 // for 16bit Command

#define DWM_READ8bit_MASK 	0x00 // for 8bit Command
#define DWM_WRITE8bit_MASK 	0x80 // for 8bit Command

#define DWM_8BIT_MASK 	0x0000
#define DWM_16BIT_MASK  0x4000

#define DWM_RXDATA_FILE_ID		0x12
#define DWM_RX_BUFFER_0				0x00	// RX frame data buffer 0


void Init(SPI_HandleTypeDef *hspi, uint8_t *rxbuffer);
void DWM3000_Read_RX_Data(void);

#endif /* INC_MYDWM3000_H_ */
