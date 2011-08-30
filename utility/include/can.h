// coding: utf-8
// ----------------------------------------------------------------------------
/*
 * Copyright (c) 2011 D.E. Goodman-Wilson
 *  Based on code from the following:
 *      [this header file] Copyright (c) 2007 Fabian Greif, Roboterclub Aachen e.V.
 *      [the source files] Copyright (c) 2011 Per Eklund
 *
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
// ----------------------------------------------------------------------------
/**
 * \file    can.h
 * \brief   Header definition for CAN Interface
 */
// ----------------------------------------------------------------------------

// $Id: can.h 1198 2011-04-03 01:16:46Z jacobsen $ 


#include "lpc17xx_pinsel.h"
#include "lpc17xx_can.h"

#ifndef CAN_H
#define CAN_H

//DEG 18 Aug 2011. Useful defaults?
#ifndef CAN_RX_BUFFER_SIZE
#define CAN_RX_BUFFER_SIZE 20
#endif

#ifndef CAN_TX_BUFFER_SIZE
#define CAN_TX_BUFFER_SIZE 0
#endif


#if defined (__cplusplus)
extern "C" {
#endif

void CAN_IRQHandler(void);


// ----------------------------------------------------------------------------
/**
 * \ingroup		communication
 * \defgroup 	can_interface Universelles CAN Interface
 * \brief		allgemeines CAN Interface für AT90CAN32/64/128, MCP2515 und SJA1000
 *
 * \author 		Fabian Greif <fabian.greif@rwth-aachen.de>
 * \author      Roboterclub Aachen e.V. (http://www.roboterclub.rwth-aachen.de)
 *
 * \version		$Id: can.h 1198 2011-04-03 01:16:46Z jacobsen $
 */
// ----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

// ----------------------------------------------------------------------------
/** \ingroup	can_interface
 *  \name		Bitdefinitionen
 */
//@{
#define	ONLY_NON_RTR		2
#define	ONLY_RTR		    3
//@}

/** \ingroup	can_interface
 *  \name		Bitraten fuer den CAN-Bus 
 */
//@{
#define	BITRATE_10_KBPS		0	// ungetestet
#define	BITRATE_20_KBPS		1	// ungetestet
#define	BITRATE_50_KBPS		2	// ungetestet
#define	BITRATE_100_KBPS	3	// ungetestet
#define	BITRATE_125_KBPS	4
#define	BITRATE_250_KBPS	5
#define	BITRATE_500_KBPS	6
#define	BITRATE_1_MBPS		7
//@}

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Datenstruktur zum Aufnehmen von CAN Nachrichten
 */
typedef struct
{
    uint32_t id; //!< ID der Nachricht (11 oder 29 Bit)
    struct
    {
        int rtr :1; //!< Remote-Transmit-Request-Frame?
        int extended :1; //!< extended ID?
    } flags;

    uint8_t length; //!< Anzahl der Datenbytes
    uint8_t data[8]; //!< Die Daten der CAN Nachricht

} tCAN;

// ----------------------------------------------------------------------------
/**
 * \ingroup can_interface
 * \brief   Inhalt der Fehler-Register
 */
typedef struct
{
    uint8_t rx; //!< Empfangs-Register
    uint8_t tx; //!< Sende-Register
} tCANErrorRegister;

// ----------------------------------------------------------------------------
/**
 * \ingroup can_interface
 * \brief   Modus des CAN Interfaces
 */
typedef enum
{
    LISTEN_ONLY_MODE, //!< der CAN Contoller empfängt nur und verhält sich völlig passiv
    LOOPBACK_MODE, //!< alle Nachrichten direkt auf die Empfangsregister umleiten ohne sie zu senden
    NORMAL_MODE
//!< normaler Modus, CAN Controller ist aktiv
} tCANMode;

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Initialisierung des CAN Interfaces
 *
 * \param	bitrate	Gewuenschte Geschwindigkeit des CAN Interfaces
 *
 * \return	false falls das CAN Interface nicht initialisiert werden konnte,
 *			true ansonsten.
 */
bool can_init(uint8_t bitrate);


// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Ueberpruefen ob neue CAN Nachrichten vorhanden sind
 *
 * \return	true falls neue Nachrichten verfuegbar sind, false ansonsten.
 */
bool can_check_message(void);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Ueberprueft ob ein Puffer zum Versenden einer Nachricht frei ist.
 *
 * \return	true falls ein Sende-Puffer frei ist, false ansonsten.
 */
bool can_check_free_buffer(void);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Verschickt eine Nachricht über den CAN Bus
 *
 * \param	msg	Nachricht die verschickt werden soll
 * \return	FALSE falls die Nachricht nicht verschickt werden konnte, \n
 *			ansonsten der Code des Puffes in den die Nachricht gespeichert wurde
 */
uint8_t can_send_message(const tCAN *msg);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Liest eine Nachricht aus den Empfangspuffern des CAN Controllers
 *
 * \param	msg	Pointer auf die Nachricht die gelesen werden soll.
 * \return	FALSE falls die Nachricht nicht ausgelesen konnte,
 *			ansonsten Filtercode welcher die Nachricht akzeptiert hat.
 */
uint8_t can_get_message(tCAN *msg);


#if defined (__cplusplus)
}
#endif

#endif // CAN_H
