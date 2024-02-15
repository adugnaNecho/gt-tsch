/**
 * This file contains the code for creating the TSCH schedule in the GT-TSCH protocol.
 * Developed by Omid Tavallaie.
 * 
 * The GT-TSCH protocol aims to enhance synchronous communication in low-power IoT networks
 * by implementing efficient time-slotted channel hopping mechanisms. This client
 * code is designed to run on Contiki-NG operating system, leveraging its networking
 * stack to demonstrate the protocol's capabilities in a practical scenario.
 */
 
 
#ifndef _DTSF_SF_SIMPLE_H_
#define _DTSF_SF_SIMPLE_H_

#include "net/linkaddr.h"
#include "os/net/mac/tsch/sixtop/sixtop.h"

 int dtsf_ask_adv_link(const linkaddr_t *peer_addr, uint32_t number_of_links);
 int dtsf_ask_channel(const linkaddr_t *peer_addr);
 int select_frequency_for_children();
 
void dtsf_send_add_downlink();
void  dtsf_send_delete_downlink(const linkaddr_t* peer_addr, sf_simple_cell_t* cell_list, uint16_t cell_list_len);
int dtsf_send_add_uplink(const linkaddr_t *peer_addr, uint32_t number_of_links);
void  dtsf_send_delete_uplink(const linkaddr_t* peer_addr, sf_simple_cell_t* cell_list, uint16_t cell_list_len);

extern int check_adv_link;
#define SF_SIMPLE_MAX_LINKS  20
#define SF_SIMPLE_SFID       0x00
extern const sixtop_sf_t sf_simple_driver;

#endif /* !_SIXTOP_SF_SIMPLE_H_ */
