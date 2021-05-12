/*! ----------------------------------------------------------------------------
 *  @file    main.c
 *  @brief   TX then wait for response example code
 *
 *           This is a simple code example that sends a frame and then turns on the DW1000 receiver to receive a response. The response could be
 *           anything as no check is performed on it and it is only displayed in a local buffer but the sent frame is the one expected by the
 *           companion simple example "RX then send a response example code". After the response is received or if the reception timeouts, this code
 *           just go back to the sending of a new frame.
 *
 * @attention
 *
 * Copyright 2015 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */
#include "deca_device_api.h"
#include "deca_regs.h"
//#include "sleep.h"
//#include "lcd.h"
//#include "port.h"
#include <stdio.h>
#include "dw1000.h"

/* Example application name and version to display on LCD screen. */
#define APP_NAME "TX WAITRESP v1.2"

/* Default communication configuration. We use here EVK1000's default mode (mode 3). */
static dwt_config_t config = {
    5,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
    DWT_PLEN_128  ,   /* Preamble length. Used in TX only. */
    DWT_PAC8,       /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    0,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_6M8,     /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    (129 + 8 - 8)  /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};


/* The frame sent in this example is a blink encoded as per the ISO/IEC 24730-62:2013 standard. It is a 14-byte frame composed of the following fields:
 *     - byte 0: frame control (0xC5 to indicate a multipurpose frame using 64-bit addressing).
 *     - byte 1: sequence number, incremented for each new frame.
 *     - byte 2 -> 9: device ID, see NOTE 1 below.
 *     - byte 10: encoding header (0x43 to indicate no extended ID, temperature, or battery status is carried in the message).
 *     - byte 11: EXT header (0x02 to indicate tag is listening for a response immediately after this message).
 *     - byte 12/13: frame check-sum, automatically set by DW1000. */
static uint8 tx_msg[] = {7,4,6,0,1,6,3,2,2,2,3,1,2,2,2,3,3,2,3,3,4,3,3,4,4,2,3,3,4,4,4,4,4,4,5,5,6,4,4,4,4,5,6,5,6,7,7,8,8,8,9};
/* Index to access to sequence number of the blink frame in the tx_msg array. */
#define BLINK_FRAME_SN_IDX 1

/* Inter-frame delay period, in milliseconds. */
#define TX_DELAY_MS 30

/* Delay from end of transmission to activation of reception, expressed in UWB microseconds (1 uus is 512/499.2 microseconds). See NOTE 2 below. */
#define TX_TO_RX_DELAY_UUS 60

/* Receive response timeout, expressed in UWB microseconds. See NOTE 3 below. */
#define RX_RESP_TO_UUS 0

/* Buffer to store received frame. See NOTE 4 below. */
#define FRAME_LEN_MAX 255
static uint8 rx_buffer[FRAME_LEN_MAX];

/* Hold copy of status register state here for reference so that it can be examined at a debug breakpoint. */
static uint32 status_reg = 0;

/* Hold copy of frame length of frame received (if good) so that it can be examined at a debug breakpoint. */
static uint16 frame_len = 0;

/**
 * Application entry point.
 */
int example_application_entry(void)
{
	
	uint32_t rx_start_time = 0, rx_end_time = 0;
  uint32_t rx_spend_time = 0;

//    /* Start with board specific hardware init. */
//    peripherals_init();

//    /* Display application name on LCD. */
//    lcd_display_str(APP_NAME);
     printf("%s\r\n",APP_NAME);
    /* Reset and initialise DW1000. See NOTE 5 below.
     * For initialisation, DW1000 clocks must be temporarily set to crystal speed. After initialisation SPI rate can be increased for optimum
     * performance. */
    reset_DW1000(); /* Target specific drive of RSTn line into DW1000 low for a period. */
    spi_set_rate_low();
    if (dwt_initialise(DWT_LOADNONE) == DWT_ERROR)
    {
        printf("INIT FAILED\r\n");
        while (1)
        { };
    }
		printf("INIT OK\r\n");
    spi_set_rate_high();

    /* Configure DW1000. See NOTE 6 below. */
    dwt_configure(&config);

    /* Set delay to turn reception on after transmission of the frame. See NOTE 2 below. */
    dwt_setrxaftertxdelay(TX_TO_RX_DELAY_UUS);

    /* Set response frame timeout. */
    dwt_setrxtimeout(RX_RESP_TO_UUS);

    /* Loop forever sending and receiving frames periodically. */
    while (1)
    {
        /* Write frame data to DW1000 and prepare transmission. See NOTE 7 below. */
        dwt_writetxdata(sizeof(tx_msg), tx_msg, 0); /* Zero offset in TX buffer. */
        dwt_writetxfctrl(sizeof(tx_msg), 0, 0); /* Zero offset in TX buffer, no ranging. */

        /* Start transmission, indicating that a response is expected so that reception is enabled immediately after the frame is sent. */
        dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
			/* get start timestamp */
      rx_start_time = HAL_GetTick();

			printf("send ok\r\n");

      printf("send data:\r\n[");
/* hex dump */
  for (int i = 0; i < sizeof(tx_msg); i++)
{
    printf("%02x ", tx_msg[i]);
}
   printf("]\r\n");


        /* We assume that the transmission is achieved normally, now poll for reception of a frame or error/timeout. See NOTE 8 below. */
//        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
//        { };

        if (status_reg & SYS_STATUS_RXFCG)
        {
					/* get end timestamp */
            rx_end_time = HAL_GetTick();

            int i;

            /* Clear local RX buffer to avoid having leftovers from previous receptions. This is not necessary but is included here to aid reading
             * the RX buffer. */
            for (i = 0 ; i < FRAME_LEN_MAX; i++ )
            {
                rx_buffer[i] = 0;
            }

            /* A frame has been received, copy it to our local buffer. */
            frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
            if (frame_len <= FRAME_LEN_MAX)
            {
                dwt_readrxdata(rx_buffer, frame_len, 0);
							printf("recv ok\r\n");

            printf("recv resp data:\r\n[");
          /* hex dump */
          for (int i = 0; i < frame_len; i++)
          {
             printf("%02x ", rx_buffer[i]);
             }
            printf("]\r\n");
            }

            /* TESTING BREAKPOINT LOCATION #1 */

            /* At this point, received frame can be examined in global "rx_buffer". An actual application would, for example, start by checking that
             * the format and/or data of the response are the expected ones. A developer might put a breakpoint here to examine this frame. */

            /* Clear good RX frame event in the DW1000 status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);
        }
        else
        {      if (status_reg & SYS_STATUS_ALL_RX_TO){
					  printf("recv fail,rx timeout\r\n");
				}else
				{printf("recv fail,error");}
            /* Clear RX error/timeout events in the DW1000 status register. */
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        }
     printf("finish,%d\r\n",tx_msg[BLINK_FRAME_SN_IDX]);
				
				/* log rx spend time */
           rx_spend_time = rx_end_time - rx_start_time;
          printf("rx spend time is %d ms\r\n", rx_spend_time);

				
        /* Execute a delay between transmissions. */
        HAL_Delay(TX_DELAY_MS);

        /* Increment the blink frame sequence number (modulo 256). */
        tx_msg[BLINK_FRAME_SN_IDX]++;
    }
}

/*****************************************************************************************************************************************************
 * NOTES:
 *
 * 1. The device ID is a hard coded constant in the blink to keep the example simple but for a real product every device should have a unique ID.
 *    For development purposes it is possible to generate a DW1000 unique ID by combining the Lot ID & Part Number values programmed into the
 *    DW1000 during its manufacture. However there is no guarantee this will not conflict with someone else’s implementation. We recommended that
 *    customers buy a block of addresses from the IEEE Registration Authority for their production items. See "EUI" in the DW1000 User Manual.
 * 2. TX to RX delay can be set to 0 to activate reception immediately after transmission. But, on the responder side, it takes time to process the
 *    received frame and generate the response (this has been measured experimentally to be around 70 µs). Using an RX to TX delay slightly less than
 *    this minimum turn-around time allows the application to make the communication efficient while reducing power consumption by adjusting the time
 *    spent with the receiver activated.
 * 3. This timeout is for complete reception of a frame, i.e. timeout duration must take into account the length of the expected frame. Here the value
 *    is arbitrary but chosen large enough to make sure that there is enough time to receive a complete frame sent by the "RX then send a response"
 *    example at the 110k data rate used (around 3 ms).
 * 4. In this example, maximum frame length is set to 127 bytes which is 802.15.4 UWB standard maximum frame length. DW1000 supports an extended frame
 *    length (up to 1023 bytes long) mode which is not used in this example.
 * 5. In this example, LDE microcode is not loaded upon calling dwt_initialise(). This will prevent the IC from generating an RX timestamp. If
 *    time-stamping is required, DWT_LOADUCODE parameter should be used. See two-way ranging examples (e.g. examples 5a/5b).
 * 6. In a real application, for optimum performance within regulatory limits, it may be necessary to set TX pulse bandwidth and TX power, (using
 *    the dwt_configuretxrf API call) to per device calibrated values saved in the target system or the DW1000 OTP memory.
 * 7. dwt_writetxdata() takes the full size of tx_msg as a parameter but only copies (size - 2) bytes as the check-sum at the end of the frame is
 *    automatically appended by the DW1000. This means that our tx_msg could be two bytes shorter without losing any data (but the sizeof would not
 *    work anymore then as we would still have to indicate the full length of the frame to dwt_writetxdata()).
 * 8. We use polled mode of operation here to keep the example as simple as possible but all status events can be used to generate interrupts. Please
 *    refer to DW1000 User Manual for more details on "interrupts".
 * 9. The user is referred to DecaRanging ARM application (distributed with EVK1000 product) for additional practical example of usage, and to the
 *    DW1000 API Guide for more details on the DW1000 driver functions.
 ****************************************************************************************************************************************************/
