/*
 * Copyright (c) 2016, Yasuyuki Tanaka
 * Copyright (c) 2016, Centre for Development of Advanced Computing (C-DAC).
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file
 *         A 6P Simple Schedule Function
 * \author
 *         Shalu R <shalur@cdac.in>
 *         Lijo Thomas <lijo@cdac.in>
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
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
