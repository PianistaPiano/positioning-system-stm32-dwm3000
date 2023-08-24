/*
 * port.c
 *
 *  Created on: 19 gru 2021
 *      Author: tomas
 */

//#include "stm32l4xx_hal_def.h"
#include "main.h"
#include "port.h"
#include "core_cm4.h"



/****************************************************************************//**
 *
 *                              APP global variables
 *
 *******************************************************************************/
extern SPI_HandleTypeDef hspi3;

/****************************************************************************//**
 *
 *                  Port private variables and function prototypes
 *
 *******************************************************************************/
static volatile uint32_t signalResetDone;

typedef int (*port_dwic_isr_t)(void);
/* DW IC IRQ handler definition. */
static port_dwic_isr_t port_dwic_isr = NULL;

/****************************************************************************//**
 *
 *                              Time section
 *
 *******************************************************************************/

/* @fn    portGetTickCnt
 * @brief wrapper for to read a SysTickTimer, which is incremented with
 *        CLOCKS_PER_SEC frequency.
 *        The resolution of time32_incr is usually 1/1000 sec.
 * */
__INLINE uint32_t
portGetTickCnt(void)
{
    return HAL_GetTick();
}


/* @fn    usleep
 * @brief precise usleep() delay
 * */
#pragma GCC optimize ("O0")
int usleep(useconds_t usec)
{
    unsigned int i;

    usec*=12;
    for(i=0;i<usec;i++)
    {
        __NOP();
    }
    return 0;
}

/* @fn    Sleep
 * @brief Sleep delay in ms using SysTick timer
 * */
__INLINE void
Sleep(uint32_t x)
{
    HAL_Delay(x);
}


/****************************************************************************//**
 *
 *                              END OF Time section
 *
 *******************************************************************************/


/****************************************************************************//**
 *
 *                              IRQ section
 *
 *******************************************************************************/

/* @fn         HAL_GPIO_EXTI_Callback
 * @brief      EXTI line detection callback from HAL layer
 * @param      GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
 *             i.e. DW_RESET_Pin and DW_IRQn_Pin
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch ( GPIO_Pin )
    {
//    case DW_RST_B_Pin :
//    case DW_RST_A_Pin :
//        {
//            dw_rst_pin_irq_cb(); /* bare-metal signal */
//            break;
//        }
//
    case DW_RESET_Pin :
        signalResetDone = 1;
        break;

    case DW_IRQn_Pin :
        {
            //while (HAL_GPIO_ReadPin(DECAIRQ_GPIO, DW_IRQn_Pin) == GPIO_PIN_SET)
            {
                process_deca_irq();
                //dwt_isr();
            }

            break;
        }

    default :
        break;
    }
}

/* @fn      process_deca_irq
 * @brief   main call-back for processing of DW3000 IRQ
 *          it re-enters the IRQ routing and processes all events.
 *          After processing of all events, DW3000 will clear the IRQ line.
 * */
__INLINE void process_deca_irq(void)
{
    while(port_CheckEXT_IRQ() != 0)
    {
        if(port_dwic_isr)
        {
            port_dwic_isr();
        }
    } //while DW3000 IRQ line active
}


/* @fn      port_DisableEXT_IRQ
 * @brief   wrapper to disable DW_IRQ pin IRQ
 *          in current implementation it disables all IRQ from lines 5:9
 * */
__INLINE void port_DisableEXT_IRQ(void)
{
    NVIC_DisableIRQ(EXTI0_IRQn);
}

/* @fn      port_EnableEXT_IRQ
 * @brief   wrapper to enable DW_IRQ pin IRQ
 *          in current implementation it enables all IRQ from lines 5:9
 * */
__INLINE void port_EnableEXT_IRQ(void)
{
    NVIC_EnableIRQ(EXTI0_IRQn);
}

/**
  * @brief  Checks whether the specified IRQn line is enabled or not.
  * @param  IRQn: specifies the IRQn line to check.
  * @return "0" when IRQn is "not enabled" and !0 otherwise
  */
ITStatus EXTI_GetITEnStatus(IRQn_Type IRQn)
{
        return ((NVIC->ISER[(((uint32_t)(int32_t)IRQn) >> 5UL)] &\
            (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL)) ) == (uint32_t)RESET)?(RESET):(SET);
}

/* @fn      port_GetEXT_IRQStatus
 * @brief   wrapper to read a DW_IRQ pin IRQ status
 * */
__INLINE uint32_t port_GetEXT_IRQStatus(void)
{
    return EXTI_GetITEnStatus(EXTI0_IRQn);
}


/* @fn      port_CheckEXT_IRQ
 * @brief   wrapper to read DW_IRQ input pin state
 * */
__INLINE uint32_t port_CheckEXT_IRQ(void)
{
    return HAL_GPIO_ReadPin(DW_IRQn_GPIO_Port, DW_IRQn_Pin);
}


/****************************************************************************//**
 *
 *                              END OF IRQ section
 *
 *******************************************************************************/

/****************************************************************************//**
 *
 *                          DW IC port section
 *
 *******************************************************************************/

/* @fn      reset_DW IC
 * @brief   DW_RESET pin on DW IC has 2 functions
 *          In general it is output, but it also can be used to reset the digital
 *          part of DW IC by driving this pin low.
 *          Note, the DW_RESET pin should not be driven high externally.
 * */
void reset_DWIC(void)
{
    GPIO_InitTypeDef    GPIO_InitStruct;

    // Enable GPIO used for DW1000 reset as open collector output
    GPIO_InitStruct.Pin = DW_RESET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DW_RESET_GPIO_Port, &GPIO_InitStruct);

    //drive the RSTn pin low
    HAL_GPIO_WritePin(DW_RESET_GPIO_Port, DW_RESET_Pin, GPIO_PIN_RESET);

    usleep(1);

    //put the pin back to output open-drain (not active)
    setup_DWICRSTnIRQ(0);
    Sleep(2);

}
void my_reset_DWIC(void)
{
	HAL_GPIO_WritePin(DW_RESET_GPIO_Port, DW_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(6);
	HAL_GPIO_WritePin(DW_RESET_GPIO_Port, DW_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(3);
	Sleep(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC)
}

/* @fn      setup_DWICRSTnIRQ
 * @brief   setup the DW_RESET pin mode
 *          0 - output Open collector mode
 *          !0 - input mode with connected EXTI0 IRQ
 * */
void setup_DWICRSTnIRQ(int enable)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    if(enable)
    {
        // Enable GPIO used as DECA RESET for interrupt
        GPIO_InitStruct.Pin = DW_RESET_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(DW_RESET_GPIO_Port, &GPIO_InitStruct);

        HAL_NVIC_EnableIRQ(EXTI0_IRQn);     //pin #0 -> EXTI #0
        HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    }
    else
    {
        HAL_NVIC_DisableIRQ(EXTI0_IRQn);    //pin #0 -> EXTI #0

        //put the pin back to tri-state ... as
        //output open-drain (not active)
        GPIO_InitStruct.Pin = DW_RESET_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(DW_RESET_GPIO_Port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(DW_RESET_GPIO_Port, DW_RESET_Pin, GPIO_PIN_SET);
    }
}

/* @fn      port_set_dw_ic_spi_slowrate
 * @brief   set 4.5MHz
 *          note: hspi3 is clocked from 72MHz
 * */
void port_set_dw_ic_spi_slowrate(void)
{
	hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    HAL_SPI_Init(&hspi3);
}

/* @fn      port_set_dw_ic_spi_fastrate
 * @brief   set 18MHz
 *          note: hspi3 is clocked from 72MHz
 * */
void port_set_dw_ic_spi_fastrate(void)
{
	hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    HAL_SPI_Init(&hspi3);
}

/****************************************************************************//**
 *
 *                          End APP port section
 *
 *******************************************************************************/


