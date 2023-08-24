/*
 * myFunctions.h
 *
 *  Created on: Dec 3, 2022
 *      Author: Mati
 */

#ifndef INC_MYFUNCTIONS_H_
#define INC_MYFUNCTIONS_H_

#include "main.h"
#include <string.h>
#include <stdio.h>

void split_dist_rssi(char* msg_to_PC, char* dist, char* rssi, char tag);
void get_msg_toPC(char * a, uint8_t* b, uint8_t anchor_idx);
uint8_t get_msg_toPC_2(char * a, uint8_t* b, uint8_t anchor_idx);
void add_dist_to_PCmsg(uint8_t *a, char* b, uint8_t dist_idx);
uint8_t allTrue(uint8_t * table, uint8_t numOfAnchors);
void concatenateDistancesAndRssiToString(float* distances, float* rssi, char* allDistancesToPC, uint8_t numOfAnchors, char* tag);

#endif /* INC_MYFUNCTIONS_H_ */
