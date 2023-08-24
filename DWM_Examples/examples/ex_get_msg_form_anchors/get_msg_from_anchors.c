/*! ----------------------------------------------------------------------------
 *  @file    ds_twr_responder_sts.c
 *  @brief   Double-sided two-way ranging (DS TWR) responder example code
 *
 *           This is a simple code example which acts as the responder in a DS TWR distance measurement exchange. This application waits for a "poll"
 *           message (recording the RX time-stamp of the poll) expected from the "ds_twr_initiator_v2" example code (companion to this application), and
 *           then sends a "response" message to complete the exchange. The response message contains all the time-stamps recorded by this application,
 *           including the calculated/predicted TX time-stamp for the response message itself. The companion "DS TWR initiator" example application
 *           works out the time-of-flight over-the-air and, thus, the estimated distance between the two devices.
 *
 *           This example utilises the 802.15.4z STS to accomplish secure timestamps between the initiator and responder. A 32-bit STS counter
 *           is part of the STS IV used to generate the scrambled timestamp sequence (STS) in the transmitted packet and to cross correlate in the
 *           receiver. This count normally advances by 1 for every 1024 chips (~2�s) of STS in BPRF mode, and by 1 for every 5124 chips (~1�s) of STS
 *           in HPRF mode. If both devices (initiator and responder) have count values that are synced, then the communication between devices should
 *           result in secure timestamps which can be used to calculate distance. If not, then the devices need to re-sync their STS counter values.
 *           In this example, the initiator will send a plain-text value of it's 32-bit STS counter inside the "poll" frame. The receiver first
 *           checks the quality of the STS of the received frame. If the received frame has bad STS quality, it can then use the plain-text
 *           counter value received to adjust it's own STS counter value to match. This means that the next message in the sequence should be in sync again.
 *
 * @attention
 *
 * Copyright 2019 - 2020 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */
#include "main.h"
#include "tim.h"
#include <stdlib.h>
#include <deca_device_api.h>
#include <deca_regs.h>
#include <deca_spi.h>
#include <port.h>
#include <shared_defines.h>
#include <shared_functions.h>
#include <example_selection.h>
#include <config_options.h>
#include "string.h"
#include "stdio.h"
#include "myFunctions.h"

#if defined(TEST_GET_MSG_FROM_ANCHORS)

extern void test_run_info(unsigned char *data);

/* Example application name */
#define APP_NAME "GET_MSG_FROM_ANCHORS v1.0"


/* Frames used in the ranging process. See NOTE 3 below. */
static uint8_t tx_dist_to_PC[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'P', 'C', 'A', 'N', 0xE2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 26 length
//                                0      1    2  3      4     5    6    7    8    9    10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
/* Length of the common part of the message (up to and including the function code, see NOTE 3 below). */
#define ALL_MSG_COMMON_LEN 10
#define ANCHOR_IDX 10
#define ALL_MSG_SN_IDX 2

/* Buffer to store received messages.
 * Its size is adjusted to longest frame that this example code is supposed to handle. */
#define RX_BUF_LEN 24//Must be less than FRAME_LEN_MAX_EX
static uint8_t rx_buffer[RX_BUF_LEN];

/* Hold copy of status register state here for reference so that it can be examined at a debug breakpoint. */
static uint32_t status_reg = 0;


/* Hold the amount of errors that have occurred */
static uint32_t errors[23] = {0};

char msg_to_PC[16];

extern dwt_config_t config_options;
extern dwt_txconfig_t txconfig_options;
extern dwt_txconfig_t txconfig_options_ch9;

/*
 * 128-bit STS key to be programmed into CP_KEY register.
 *
 * This key needs to be known and programmed the same at both units performing the SS-TWR.
 * In a real application for security this would be private and unique to the two communicating units
 * and chosen/assigned in a secure manner lasting just for the period of their association.
 *
 * Here we use a default KEY as specified in the IEEE 802.15.4z annex
 */
static dwt_sts_cp_key_t cp_key =
{
        0x14EB220F,0xF86050A8,0xD1D336AA,0x14148674
};

/*
 * 128-bit initial value for the nonce to be programmed into the CP_IV register.
 *
 * The IV, like the key, needs to be known and programmed the same at both units performing the SS-TWR.
 * It can be considered as an extension of the KEY. The low 32 bits of the IV is the counter.
 * In a real application for any particular key the value of the IV including the count should not be reused,
 * i.e. if the counter value wraps the upper 96-bits of the IV should be changed, e.g. incremented.
 *
 * Here we use a default IV as specified in the IEEE 802.15.4z annex
 */
static dwt_sts_cp_iv_t cp_iv =
{
        0x1F9A3DE4,0xD37EC3CA,0xC44FA8FB,0x362EEB34
};

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn ds_twr_responder_sts()
 *
 * @brief Application entry point.
 *
 * @param  none
 *
 * @return none
 */
int get_msg_from_anchors(void)
{
    int16_t stsQual; /* This will contain STS quality index and status */
    int goodSts = 0; /* Used for checking STS quality in received signal */
    uint8_t messageFlag = 0; /* Used to track whether STS count should be reinitialised or not */
    /* Display application name on UART. */
    test_run_info((unsigned char *)APP_NAME);


    /* Reset DW IC */
    my_reset_DWIC();

    while (!dwt_checkidlerc()) /* Need to make sure DW IC is in IDLE_RC before proceeding */
    { };

    // Start timer from STM
    //HAL_TIM_Base_Start(&htim2);

    if (dwt_initialise(DWT_DW_IDLE) == DWT_ERROR)
    {
        test_run_info((unsigned char *)"INIT FAILED     ");
        while (1)
        { };
    }

    /* Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield boards.
     * Note, in real low power applications the LEDs should not be used. */
    //dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK) ;

    /* Configure DW IC. See NOTE 14 below. */
    if(dwt_configure(&config_options)) /* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device */
    {
        test_run_info((unsigned char *)"CONFIG FAILED     ");
        while (1)
        { };
    }

    /* Configure the TX spectrum parameters (power, PG delay and PG count) */
    if(config_options.chan == 5)
    {
        dwt_configuretxrf(&txconfig_options);
    }
    else
    {
        dwt_configuretxrf(&txconfig_options_ch9);
    }

    //dwt_setrxtimeout(100000);
    //dwt_setpreambledetecttimeout(2); //nic wtedy nie łapie
    //configure frame filtering
    dwt_configureframefilter(DWT_FF_ENABLE_802_15_4, DWT_FF_DATA_EN);
    dwt_setpanid(0xDECA);
    dwt_setaddress16(0x4350);


    /* Loop responding to ranging requests, for RANGE_COUNT number of times */
    //while (loopCount < RANGE_COUNT)
	while(1)
    {

		dwt_rxenable(DWT_START_RX_IMMEDIATE);

        /* Poll for reception of a frame or error/timeout. See NOTE 6 below. */
        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
        { };

        /*
         * Check for a good frame
         */
        if ((status_reg & SYS_STATUS_RXFCG_BIT_MASK))
        {
        	//test_run_info((unsigned char *)"Jestem1");
            uint32_t frame_len;

            /* Clear good RX frame event in the DW IC status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

            /* A frame has been received, read it into the local buffer. */
            frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
            if (frame_len <= sizeof(rx_buffer))
            {
            	//test_run_info((unsigned char *)"Jestem2");
                dwt_readrxdata(rx_buffer, frame_len, 0);

                /* Check that the frame is a poll sent by "SS TWR initiator STS" example.
                 * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
                rx_buffer[ALL_MSG_SN_IDX] = 0;
                if (memcmp(rx_buffer, tx_dist_to_PC, ALL_MSG_COMMON_LEN) == 0)
                {
                	get_msg_toPC(msg_to_PC, rx_buffer, ANCHOR_IDX);
                	test_run_info((unsigned char *)msg_to_PC);
                	memset(msg_to_PC, 0, 10*sizeof(char));
                }
                else
                {
                	//test_run_info((unsigned char *)"Jestem4");
                    errors[BAD_FRAME_ERR_IDX] += 1;
                    /*
                     * If any error occurs, we can reset the STS count back to default value.
                     */
                    messageFlag = 0;
                }
            }
            else
            {
            	//test_run_info((unsigned char *)"Jestem5");
                errors[RTO_ERR_IDX] += 1;
                /*
                 * If any error occurs, we can reset the STS count back to default value.
                 */
                messageFlag = 0;
            }
        }
        else
        {
        	//test_run_info((unsigned char *)"Jestem6");
#if 0
            check_for_status_errors(status_reg, errors);

            if (!(status_reg & SYS_STATUS_RXFCG_BIT_MASK))
            {
                errors[BAD_FRAME_ERR_IDX] += 1;
            }
            if (goodSts < 0)
            {
                errors[PREAMBLE_COUNT_ERR_IDX] += 1;
            }
            if (stsQual <= 0)
            {
                errors[CP_QUAL_ERR_IDX] += 1;
            }
#endif
            /* Clear RX error events in the DW IC status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);

            /*
             * If any error occurs, we can reset the STS count back to default value.
             */
            messageFlag = 0;
        }
        //HAL_Delay(10);

    }
    return DWT_SUCCESS;
}
#endif
/*****************************************************************************************************************************************************
 * NOTES:
 *
 * 1. The double-sided two-way ranging scheme implemented here has to be considered carefully as the accuracy of the distance measured is highly
 *    sensitive to the clock offset error between the devices and the length of the response delay between frames. To achieve the best possible
 *    accuracy, this response delay must be kept as low as possible. In order to do so, 6.8 Mbps data rate is advised in this example and the response
 *    delay between frames is defined as low as possible. The user is referred to User Manual for more details about the double-sided two-way ranging
 *    process.
 *
 *    Initiator: |Poll TX| ..... |Resp RX| ........ |Final TX|
 *    Responder: |Poll RX| ..... |Resp TX| ........ |Final RX|
 *                   ^|P RMARKER|                                    - time of Poll TX/RX
 *                                   ^|R RMARKER|                    - time of Resp TX/RX
 *                                                      ^|R RMARKER| - time of Final TX/RX
 *
 *                       <--TDLY->                                   - POLL_TX_TO_RESP_RX_DLY_UUS (RDLY-RLEN)
 *                               <-RLEN->                            - RESP_RX_TIMEOUT_UUS   (length of poll frame)
 *                    <----RDLY------>                               - POLL_RX_TO_RESP_TX_DLY_UUS (depends on how quickly responder
 *                                                                                                                      can turn around and reply)
 *
 *
 *                                        <--T2DLY->                 - RESP_TX_TO_FINAL_RX_DLY_UUS (R2DLY-FLEN)
 *                                                  <-FLEN--->       - FINAL_RX_TIMEOUT_UUS   (length of response frame)
 *                                    <----RDLY--------->            - RESP_RX_TO_FINAL_TX_DLY_UUS (depends on how quickly initiator
 *                                                                                                                      can turn around and reply)
 *
 *
 * 2. The sum of the values is the TX to RX antenna delay, experimentally determined by a calibration process. Here we use a hard coded typical value
 *    but, in a real application, each device should have its own antenna delay properly calibrated to get the best possible precision when performing
 *    range measurements.
 * 3. The frames used here are Decawave specific ranging frames, complying with the IEEE 802.15.4 standard data frame encoding. The frames are the
 *    following:
 *     - a poll message sent by the initiator to trigger the ranging exchange.
 *     - a response message sent by the responder to complete the exchange and provide all information needed by the initiator to compute the
 *       time-of-flight (distance) estimate.
 *    The first 10 bytes of those frame are common and are composed of the following fields:
 *     - byte 0/1: frame control (0x8841 to indicate a data frame using 16-bit addressing).
 *     - byte 2: sequence number, incremented for each new frame.
 *     - byte 3/4: PAN ID (0xDECA).
 *     - byte 5/6: destination address, see NOTE 4 below.
 *     - byte 7/8: source address, see NOTE 4 below.
 *     - byte 9: function code (specific values to indicate which message it is in the ranging process).
 *    The remaining bytes are specific to each message as follows:
 *    Poll message:
 *     - no more data
 *    Response message:
 *     - byte 10 -> 13: poll message reception timestamp.
 *     - byte 14 -> 17: response message transmission timestamp.
 *    All messages end with a 2-byte checksum automatically set by DW IC.
 * 4. Source and destination addresses are hard coded constants in this example to keep it simple but for a real product every device should have a
 *    unique ID. Here, 16-bit addressing is used to keep the messages as short as possible but, in an actual application, this should be done only
 *    after an exchange of specific messages used to define those short addresses for each device participating to the ranging exchange.
 * 5. In a real application, for optimum performance within regulatory limits, it may be necessary to set TX pulse bandwidth and TX power, (using
 *    the dwt_configuretxrf API call) to per device calibrated values saved in the target system or the DW IC OTP memory.
 * 6. We use polled mode of operation here to keep the example as simple as possible but all status events can be used to generate interrupts. Please
 *    refer to DW IC User Manual for more details on "interrupts". It is also to be noted that STATUS register is 5 bytes long but, as the event we
 *    use are all in the first bytes of the register, we can use the simple dwt_read32bitreg() API call to access it instead of reading the whole 5
 *    bytes.
 * 7. As we want to send final TX timestamp in the final message, we have to compute it in advance instead of relying on the reading of DW IC
 *    register. Timestamps and delayed transmission time are both expressed in device time units so we just have to add the desired response delay to
 *    response RX timestamp to get final transmission time. The delayed transmission time resolution is 512 device time units which means that the
 *    lower 9 bits of the obtained value must be zeroed. This also allows to encode the 40-bit value in a 32-bit words by shifting the all-zero lower
 *    8 bits.
 * 8. In this operation, the high order byte of each 40-bit timestamps is discarded. This is acceptable as those time-stamps are not separated by
 *    more than 2**32 device time units (which is around 67 ms) which means that the calculation of the round-trip delays (needed in the
 *    time-of-flight computation) can be handled by a 32-bit subtraction.
 * 9. dwt_writetxdata() takes the full size of the message as a parameter but only copies (size - 2) bytes as the check-sum at the end of the frame is
 *    automatically appended by the DW IC. This means that our variable could be two bytes shorter without losing any data (but the sizeof would not
 *    work anymore then as we would still have to indicate the full length of the frame to dwt_writetxdata()).
 * 10. When running this example on the Dw3000 eval board platform with the POLL_RX_TO_RESP_TX_DLY response delay provided, the dwt_starttx() is always
 *     successful. However, in cases where the delay is too short (or something else interrupts the code flow), then the dwt_starttx() might be issued
 *     too late for the configured start time. The code below provides an example of how to handle this condition: In this case it abandons the
 *     ranging exchange and simply goes back to awaiting another poll message. If this error handling code was not here, a late dwt_starttx() would
 *     result in the code flow getting stuck waiting subsequent RX event that will will never come. The companion "initiator" example (ex_06a) should
 *     timeout from awaiting the "response" and proceed to send another poll in due course to initiate another ranging exchange.
 * 11. The user is referred to DecaRanging ARM application (distributed with EVK1000 product) for additional practical example of usage, and to the
 *     DW IC API Guide for more details on the DW IC driver functions.
 * 12. In this example, the DW IC is put into IDLE state after calling dwt_initialise(). This means that a fast SPI rate of up to 20 MHz can be used
 *     thereafter.
 * 13. This example uses STS with a packet configuration of mode 1 which looks like so:
 *    ---------------------------------------------------
 *    | Ipatov Preamble | SFD | STS | PHR | PHY Payload |
 *    ---------------------------------------------------
 *    There is a possibility that the TX and RX units in this example will go out of sync as their STS IV values may be misaligned. The STS IV value
 *    changes upon each receiving and transmitting event by the chip. While the TX and RX devices in this example start at the same STS IV values, it
 *    is possible that they can go out sync if a signal is not received correctly, devices are out of range, etc. To combat this, the 'poll message'
 *    that the initiator sends to the responder contains a plain-text STS counter value. The responder receives this message and first checks if
 *    the received frame is out of sync with it's own counter. If so, it will use this received counter value to update it's own counter. When out
 *    of sync with each other, the STS will not align correctly - thus we get no secure timestamp values.
 * 14. Desired configuration by user may be different to the current programmed configuration. dwt_configure is called to set desired
 *     configuration.
 * 15. In this operation, the high order byte of each 40-bit timestamps is discarded. This is acceptable as those time-stamps are not separated by
 *     more than 2**32 device time units (which is around 67 ms) which means that the calculation of the round-trip delays (needed in the
 *     time-of-flight computation) can be handled by a 32-bit subtraction.
 * 16. This example will set the STS key and IV upon each iteration of the main while loop. While this has the benefit of keeping the STS count in
 *     sync with the responder device (which does the same), it should be noted that this is not a 'secure' implementation as the count is reset upon
 *     each iteration of the loop. An attacker could potentially recognise this pattern if the signal was being monitored. While it serves it's
 *     purpose in this simple example, it should not be utilised in any final solution.
 ****************************************************************************************************************************************************/
