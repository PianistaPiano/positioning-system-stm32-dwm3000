/*
 * port.h
 *
 *  Created on: 19 gru 2021
 *      Author: tomas
 */

#ifndef EXAMPLES_PORT_H_
#define EXAMPLES_PORT_H_

#include "machine/_types.h"
#include "sys/types.h"


void setup_DWICRSTnIRQ(int enable);
void port_set_dw_ic_spi_fastrate(void);
void port_set_dw_ic_spi_slowrate(void);
void Sleep(uint32_t x);
int usleep(useconds_t usec);
void reset_DWIC(void);
void my_reset_DWIC(void);
void process_deca_irq(void);
void port_DisableEXT_IRQ(void);
uint32_t port_GetEXT_IRQStatus(void);
void port_EnableEXT_IRQ(void);
uint32_t port_CheckEXT_IRQ(void);

#endif /* EXAMPLES_PORT_H_ */
