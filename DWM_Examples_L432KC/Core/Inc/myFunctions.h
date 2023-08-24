/*
 * myFunctions.h
 *
 *  Created on: Dec 3, 2022
 *      Author: Mati
 */

#ifndef INC_MYFUNCTIONS_H_
#define INC_MYFUNCTIONS_H_

#include "main.h"

void add_distANDrssi_to_PCmsg(uint8_t *a, char* b, char* c, uint8_t dist_idx, uint8_t addressTag);
float getRSSI(uint32_t C, uint16_t N, uint8_t D, float A);

#endif /* INC_MYFUNCTIONS_H_ */
