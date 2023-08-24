/*! ----------------------------------------------------------------------------
 * @file    deca_sleep.c
 * @brief   platform dependent sleep implementation
 *
 * @attention
 *
 * Copyright 2015-2020 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include <port.h>
#include "../../../decadriver/Inc/deca_device_api.h"

/* Wrapper function to be used by decadriver. Declared in deca_device_api.h */
void deca_sleep(unsigned int time_ms)
{
    Sleep(time_ms);
}


/* Wrapper function to be used by decadriver. Declared in deca_device_api.h */
void deca_usleep(unsigned long time_us)
{
    usleep(time_us);
}
