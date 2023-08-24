/*
 * myFunctions.c
 *
 *  Created on: Dec 3, 2022
 *      Author: Mati
 */


#include "myFunctions.h"

/*
 * get_msg_toPC function take from table b (message from anchor with id and distance)
 * distance while a char is not letter "m"
 * this function is used only in PC module
 */
void get_msg_toPC(char * a, uint8_t* b, uint8_t anchor_idx)
{
	uint8_t i =0;
	while( (char)b[anchor_idx + i] != 'm' )
	{
		a[i] = (char)b[anchor_idx + i];
		i++;
	}
}

uint8_t get_msg_toPC_2(char * a, uint8_t* b, uint8_t anchor_idx)
{
	uint8_t i =0;
	while( (char)b[anchor_idx + i] != 'd' )
	{
		a[i] = (char)b[anchor_idx + i];
		i++;
	}
	a[i] = (char)b[anchor_idx + i + 1];

	return i;
}
/*
 * add_dist_to_PCmsg function add distance to message which is send from anchor to PC module
 * this function is used only in anchors
 */
void add_dist_to_PCmsg(uint8_t *a, char* b, uint8_t dist_idx)
{
	for (uint8_t i = 0; i < strlen(b);  i++)
	{
		a[dist_idx + i] = b[i];
	}

}
/*
 * allTrue function check that in table are only values 1
 * this function is used only in PC module
 */
uint8_t allTrue(uint8_t * table, uint8_t numOfAnchors)
{
	if (numOfAnchors == 4)
	{
		if (table[0] && table[1] && table[2] && table[3])
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else if (numOfAnchors == 6)
	{
		if (table[0] && table[1] && table[2] && table[3] && table[4] && table[5])
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

/*
 * concatenateDistancesToString function concatenate all distances in table distances
 * to one string separate them using ":"
 * this function is used only in PC module
 */
void concatenateDistancesAndRssiToString(float* distances, float* rssi, char* allDistancesToPC, uint8_t numOfAnchors, char* tag)
{

	char distanceAndRssi[20];
	char rssi_str[10];

	for (int i = 0; i < numOfAnchors; i++)
	{
		sprintf(distanceAndRssi, "%3.2f/", distances[i]);
		sprintf(rssi_str, "%4.2f:", rssi[i]);
		strcat(distanceAndRssi, rssi_str);
		strcat(allDistancesToPC, distanceAndRssi);
	}
	allDistancesToPC[strlen(allDistancesToPC)] = tag[0];
	allDistancesToPC[strlen(allDistancesToPC)] = '\n';
	allDistancesToPC[strlen(allDistancesToPC)] = '\r';

}
/*
 * this function only works for specific string to get rssi and dist split
 */
void split_dist_rssi(char* msg_to_PC, char* dist, char* rssi, char tag)
{
	uint8_t i = 1;
	uint8_t j = 0;
	while(msg_to_PC[i] != 'm')
	{
		dist[i-1] = msg_to_PC[i];
		i++;
	}
	i++;
	while(msg_to_PC[i] != tag)
	{
		rssi[j] = msg_to_PC[i];
		i++;
		j++;
	}
}
















