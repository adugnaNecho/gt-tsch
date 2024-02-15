/*
 * Copyright (c) 2015, SICS Swedish ICT.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_


#define SICSLOWPAN_CONF_FRAG 0
#define UIP_CONF_BUFFER_SIZE 160


#define TSCH_CONF_WITH_SIXTOP 1

#define ENERGEST_CONF_ON 1

#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE (uint8_t[]){17, 23, 15, 25, 19, 11, 13, 21}


#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 32


#undef TSCH_SCHEDULE_CONF_MAX_LINKS
#define TSCH_SCHEDULE_CONF_MAX_LINKS 32

//#define TSCH_CONF_EB_PERIOD (2 * CLOCK_SECOND)
//#undef TSCH_CONF_MAX_EB_PERIOD 
#define TSCH_CONF_MAX_EB_PERIOD (2 * CLOCK_SECOND)


#define TSCH_CONF_DEFAULT_TIMESLOT_LENGTH 15000

/*
#define TSCH_PACKET_CONF_EACK_WITH_SRC_ADDR 1

#define TSCH_PACKET_CONF_EB_WITH_SLOTFRAME_AND_LINK 1


*/
/*
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES 0*/
#undef RPL_CONF_MOP
#define RPL_CONF_MOP RPL_MOP_NON_STORING 


#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0x81a5

#undef  MAC_CONF_WITH_TSCH
#define MAC_CONF_WITH_TSCH 1


#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     tschmac_driver

#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nordc_driver

#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154


#define RPL_CONF_DIO_INTERVAL_MIN 5

//These lines must be commented
/*
#undef  RPL_CALLBACK_PARENT_SWITCH 
#define RPL_CALLBACK_PARENT_SWITCH tsch_rpl_callback_parent_switch
#undef  RPL_CALLBACK_NEW_DIO_INTERVAL
#define RPL_CALLBACK_NEW_DIO_INTERVAL tsch_rpl_callback_new_dio_interval
#undef  TSCH_CALLBACK_JOINING_NETWORK
#define TSCH_CALLBACK_JOINING_NETWORK tsch_rpl_callback_joining_network
#undef  TSCH_CALLBACK_LEAVING_NETWORK
#define TSCH_CALLBACK_LEAVING_NETWORK tsch_rpl_callback_leaving_network
*/



#undef TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL
#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 1 /* No 6TiSCH minimal schedule */

/*
#undef TSCH_CONF_AUTOSTART
#define TSCH_CONF_AUTOSTART 1
*/


//#undef SYS_CTRL_CONF_OSC32K_USE_XTAL
//#define SYS_CTRL_CONF_OSC32K_USE_XTAL 1


//#undef DCOSYNCH_CONF_ENABLED
//#define DCOSYNCH_CONF_ENABLED 0

//#undef CC2420_CONF_SFD_TIMESTAMPS
//#define CC2420_CONF_SFD_TIMESTAMPS 1

//#undef WITH_SECURITY
//#define WITH_SECURITY 0


//#undef UIP_CONF_TCP
//#define UIP_CONF_TCP 0
//#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM 8
//#undef RPL_NS_CONF_LINK_NUM
//#define RPL_NS_CONF_LINK_NUM  8
#define DTSF_CONF_PACKET_GENERATION_RATE  0.33
//#undef NBR_TABLE_CONF_MAX_NEIGHBORS

#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 10
//#undef UIP_CONF_ND6_SEND_NS
//#define UIP_CONF_ND6_SEND_NS 0
//#undef SICSLOWPAN_CONF_FRAG
//#define SICSLOWPAN_CONF_FRAG 0
 
 /*
 
#undef  CSMA_CONF_MAX_FRAME_RETRIES
#define CSMA_CONF_MAX_FRAME_RETRIES 7

#undef  CSMA_CONF_MAX_BE
#define CSMA_CONF_MAX_BE  7

#undef  CSMA_CONF_MIN_BE  
#define CSMA_CONF_MIN_BE 4


#undef  CSMA_MAX_BACKOFF
#define CSMA_MAX_BACKOFF  3
*/
//#define TSCH_LOG_CONF_PER_SLOT                     1
//#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_INFO
//#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_DBG
#endif /* PROJECT_CONF_H_ */
