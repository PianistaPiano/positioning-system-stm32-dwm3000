/*
 * myFunctions.c
 *
 *  Created on: Dec 3, 2022
 *      Author: Mati
 */

#include "myFunctions.h"
#include <string.h>
#include "main.h"
#include <math.h>
#include <stdio.h>

/* @fn 		add_dist_to_PCmsg
 * @brief 	This function put measured distance to tx_dist_to_PC message
 * @param   a is uint8_t array message which is send to PC module
 * 			b is a measured distance
 * 			c is a calculated RSSI
 */
/*
 * add_dist_to_PCmsg function add distance to message which is send from anchor to PC module
 * this function is used only in anchors
 */
void add_distANDrssi_to_PCmsg(uint8_t *a, char* b, char* c,uint8_t dist_idx, uint8_t addressTag)
{
	uint8_t i = 0;
	uint8_t temp = 0;
	for (i = 0; i < strlen(b);  i++)
	{
		a[dist_idx + i] = b[i];
	}
	temp = dist_idx + i;
	for (i = 0; i < strlen(c); i++)
	{
		a[temp + i] = c[i];
	}

	a[temp + i] = addressTag;
}


/*
 * Calculate RSSI
 */
float getRSSI(uint32_t C, uint16_t N, uint8_t D, float A)
{
	float rssi;
	rssi = 10 * log10( (C * pow(2.0, 21.0))/pow((double) N, 2.0) ) + (6*D) - A;
	return rssi;
}
