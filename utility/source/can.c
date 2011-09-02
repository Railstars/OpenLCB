/*
 * can.c
 *
 *  Created on: 26 January 2011
 *      Author: Per Eklund
 */

//#include <stdint.h>
#include <string.h>
#include "can.h"
#include "can_buffer.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define CAN_DEV LPC_CAN2

#if CAN_RX_BUFFER_SIZE > 0
tCANBuffer can_rx_buffer;
tCAN can_rx_list[CAN_RX_BUFFER_SIZE];
#else
volatile uint8_t _messages_waiting;
#endif

#if CAN_TX_BUFFER_SIZE > 0
tCANBuffer can_tx_buffer;
tCAN can_tx_list[CAN_TX_BUFFER_SIZE];

volatile uint8_t _transmission_in_progress = 0;
#else
volatile uint8_t _free_buffer;          //!< Stores the numer of currently free MObs
#endif

void can_copy_buf_to_message(tCAN *msg);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       CAN_IRQ Handler, control receive message operation
 * param[in]    none
 * @return      none
 **********************************************************************/
//TODO put these somewhere else!
#define RxInt   0
#define TxInt   1
#define ErrInt  2

    void CAN_IRQHandler()
    {

//        CAN_MSG_Type RXMsg;
        uint8_t myIntStatus;

        /* Get CAN status */
        //IntStatus = CAN_IntGetStatus(CAN_DEV);
        myIntStatus = CAN_GetCTRLStatus(CAN_DEV, CANCTRL_INT_CAP);
        //check receive buffer status
        if( myIntStatus & (1<<RxInt) ) //if a message was received
        {
            //TODO blink LED here!
            // a message was received successfully
            #if CAN_RX_BUFFER_SIZE > 0
            tCAN *buf = can_buffer_get_enqueue_ptr(&can_rx_buffer);
            //CAN_ReceiveMsg(CAN_DEV,&RXMsg);

            if (buf != NULL)
            {
                // read message
                can_copy_buf_to_message( buf );

                // push it to the list
                can_buffer_enqueue(&can_rx_buffer);
            }
            else
            {
                // buffer overflow => reject message
                // FIXME inform the user
            }

            #else
            _messages_waiting++;

            #endif

        }
        else if( myIntStatus & (1<<ErrInt) ) //if an error trigged the interrupt...
        {
            //TODO turn LED on solid here.
        }
    }


/************************** PRIVATE FUNCTIONS *************************/

/*********************************************************************//**
 * @brief       Setup Acceptance Filter Table
 * @param[in]   none
 * @return      none
 * Note:        not use Group Standard Frame, just use for Explicit
 *              Standard and Extended Frame
 **********************************************************************/
CAN_ERROR CAN_SetupAFTable(void) {
    uint32_t i = 0;
    CAN_ERROR result;
    /* Set up Explicit Standard Frame Format Identifier Section
     * In this simple test, it has 16 entries ID
     */
    for (i = 0; i < 16; i++) {
        result = CAN_LoadExplicitEntry(LPC_CAN2, i + 1, STD_ID_FORMAT);
    }
    /* Set up Explicit Extended Frame Format Identifier Section
     * In this simple test, it has 16 entries ID
     */
    for (i = 0; i < 16; i++) {
        result = CAN_LoadExplicitEntry(LPC_CAN2, i << 11, EXT_ID_FORMAT);
    }
    return result;
}
/************************** PUBLIC FUNCTIONS  **************************/

bool can_init(uint8_t bitrate)
{
    PINSEL_CFG_Type PinCfg;

    /* Pin configuration on LPC1769
     * CAN1: select P0.0 as RD1. P0.1 as TD1
     * CAN2: select P2.7 as RD2, P2.8 as TD2
*/
    PinCfg.Funcnum = 2;
    PinCfg.Portnum = 0;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Pinnum = 4;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 5;
    PINSEL_ConfigPin(&PinCfg);

    //Initialize CAN_DEV
    switch(bitrate)
    {
        case BITRATE_10_KBPS:
            CAN_Init(CAN_DEV, 10240);
            break;
        case BITRATE_20_KBPS:
            CAN_Init(CAN_DEV, 25600);
            break;
        case BITRATE_50_KBPS:
            CAN_Init(CAN_DEV, 51200);
            break;
        case BITRATE_100_KBPS:
            CAN_Init(CAN_DEV, 102400);
            break;
        case BITRATE_125_KBPS:
            CAN_Init(CAN_DEV, 128000);
            break;
        case BITRATE_250_KBPS:
            CAN_Init(CAN_DEV, 256000);
            break;
        case BITRATE_500_KBPS:
            CAN_Init(CAN_DEV, 512000);
            break;
        case BITRATE_1_MBPS:
            CAN_Init(CAN_DEV, 1024000);
            break;
    }

    //Enable self-test mode
    CAN_ModeConfig(CAN_DEV, CAN_OPERATING_MODE, ENABLE);

    //Enable Interrupt
    CAN_IRQCmd(CAN_DEV, CANINT_RIE, ENABLE);

    // Don't filter any message. We listen to all messages
    CAN_SetAFMode(LPC_CANAF,CAN_AccBP);

    //Enable CAN Interrupt
    NVIC_EnableIRQ(CAN_IRQn);


    #if CAN_RX_BUFFER_SIZE > 0
    can_buffer_init( &can_rx_buffer, CAN_RX_BUFFER_SIZE, can_rx_list );
    #endif

    return true; //TODO
}


uint8_t can_send_message(const tCAN *msg)
{
    CAN_MSG_Type TXMsg;

    //Copy OpenLcb message structure to LPC17xx structure
    //TODO apply code for flags in msg

    if(msg->flags.extended)
    {
        TXMsg.format = EXT_ID_FORMAT;
    }
    else
    {
        //not implemented. Always extended format.
        TXMsg.format = EXT_ID_FORMAT;
    }

    if(msg->flags.rtr)
    {
        TXMsg.type = DATA_FRAME;
    }
    else
    {
        TXMsg.type = DATA_FRAME;
        //TXMsg.type = REMOTE_FRAME;
    }

    TXMsg.id = msg->id;
    TXMsg.len = msg->length;
    TXMsg.dataA[0] = msg->data[0];
    TXMsg.dataA[1] = msg->data[1];
    TXMsg.dataA[2] = msg->data[2];
    TXMsg.dataA[3] = msg->data[3];
    TXMsg.dataB[0] = msg->data[4];
    TXMsg.dataB[1] = msg->data[5];
    TXMsg.dataB[2] = msg->data[6];
    TXMsg.dataB[3] = msg->data[7];

    return CAN_SendMsg(CAN_DEV, &TXMsg);

}


//
// *** Section for Buffer ***
//
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Checks if there is any waiting message in the registers

bool can_check_message(void)
{
    #if CAN_RX_BUFFER_SIZE == 0
    if (_messages_waiting > 0)
        return true;
    else
        return false;
    #else
    return !can_buffer_empty( &can_rx_buffer );
    #endif
}


//
// Checks if there is any waiting message in the registers or buffer
//
uint8_t can_get_buffered_message(tCAN *msg)
{
    // get pointer to the first buffered message
    tCAN *buf = can_buffer_get_dequeue_ptr(&can_rx_buffer);

    if (buf == NULL)
        return 0;

    // copy the message
    memcpy( msg, buf, sizeof(tCAN) );

    // delete message from the queue
    can_buffer_dequeue(&can_rx_buffer);

    return 0xff;
}


uint8_t can_get_message(tCAN *msg)
{
#if CAN_RX_BUFFER_SIZE == 0
    if(_messages_waiting)
    {
        //TODO copy message into *msg
        can_copy_buf_to_message(msg);
        --_messages_waiting;
        return 0xff;
    }
    else
    {
        return 0;
    }
#else
    return can_get_buffered_message(msg);
#endif
}

// ----------------------------------------------------------------------------

bool can_check_free_buffer(void)
{
    #if CAN_TX_BUFFER_SIZE == 0
    // check if there is any free MOb
    if (_free_buffer > 0)
        return true;
    else
        return false;
    #else
    return !can_buffer_full( &can_tx_buffer );
    #endif
}

/*
bool check_free_buffer(void)
{
    #if CAN_TX_BUFFER_SIZE == 0
    // check if there is any free MOb
    if (_free_buffer > 0)
        return true;
    else
        return false;
    #else
    return !can_buffer_full( &can_tx_buffer );
    #endif
}
*/

//
// Status on buffers in LPC17xx
//
uint8_t can_buffers_status(void)
{
    /* Get CAN status */
    return CAN_GetCTRLStatus(CAN_DEV, CANCTRL_STS);

}


// ----------------------------------------------------------------------------
//
// *** Section for Buffer ***
//


void can_copy_buf_to_message(tCAN *msg)
{
    CAN_MSG_Type RXMsg;

    //
    // Get the message from buffer
    //
    CAN_ReceiveMsg(CAN_DEV,&RXMsg);

    msg->length = RXMsg.len;
    msg->id = RXMsg.id;
    msg->flags.extended = (RXMsg.format == EXT_ID_FORMAT);
    msg->flags.rtr = (RXMsg.type == REMOTE_FRAME);

    msg->data[0] = RXMsg.dataA[0];
    msg->data[1] = RXMsg.dataA[1];
    msg->data[2] = RXMsg.dataA[2];
    msg->data[3] = RXMsg.dataA[3];
    msg->data[4] = RXMsg.dataB[0];
    msg->data[5] = RXMsg.dataB[1];
    msg->data[6] = RXMsg.dataB[2];
    msg->data[7] = RXMsg.dataB[3];

}

#if defined (__cplusplus)
}
#endif
