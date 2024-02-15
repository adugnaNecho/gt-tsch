/*
 * Copyright (c) 2016, Yasuyuki Tanaka.
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
 * \addtogroup sixtop
 * @{
 */
/**
 * \file
 *         6top Protocol (6P) Packet Manipulation
 * \author
 *         Shalu R         <shalur@cdac.in>
 *         Lijo Thomas     <lijo@cdac.in>
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */
#include "contiki.h"
#include "contiki-lib.h"
#include "lib/assert.h"
#include "net/packetbuf.h"
#include "net/mac/tsch/tsch.h"

#include "sixp.h"
#include "sixp-pkt.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "6top"
#define LOG_LEVEL LOG_LEVEL_6TOP

static int32_t get_metadata_offset(sixp_pkt_type_t type, sixp_pkt_code_t code);
static int32_t get_cell_options_offset(sixp_pkt_type_t type,
                                       sixp_pkt_code_t code);
static int32_t get_num_cells_offset(sixp_pkt_type_t type, sixp_pkt_code_t code);
static int32_t get_reserved_offset(sixp_pkt_type_t type, sixp_pkt_code_t code);
static int32_t get_offset_offset(sixp_pkt_type_t type, sixp_pkt_code_t code);
static int32_t get_max_num_cells_offset(sixp_pkt_type_t type,
                                        sixp_pkt_code_t code);
static int32_t get_cell_list_offset(sixp_pkt_type_t type, sixp_pkt_code_t code);
static int32_t get_rel_cell_list_offset(sixp_pkt_type_t type,
                                        sixp_pkt_code_t code);
static int32_t get_total_num_cells_offset(sixp_pkt_type_t type,
                                          sixp_pkt_code_t code);
static int32_t get_payload_offset(sixp_pkt_type_t type,
                                  sixp_pkt_code_t code);

static int32_t
dtsf_get_cell_list_offset_for_adv_link(sixp_pkt_type_t type, sixp_pkt_code_t code);

static int32_t
dtsf_get_cell_list_offset_for_uplink(sixp_pkt_type_t type, sixp_pkt_code_t code);

/*---------------------------------------------------------------------------*/
static int32_t
get_metadata_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST) {
    return 0; /* offset */
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_cell_options_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST &&
     (code.cmd == SIXP_PKT_CMD_ADD ||
      code.cmd == SIXP_PKT_CMD_DELETE ||
      code.cmd == SIXP_PKT_CMD_RELOCATE ||
      code.cmd == SIXP_PKT_CMD_COUNT ||
      code.cmd == SIXP_PKT_CMD_LIST)) {
    return sizeof(sixp_pkt_metadata_t);
  }
  /*
  if(type == SIXP_PKT_TYPE_REQUEST && code.value == SIXP_PKT_CMD_ADD_ADV)
  {
     return sizeof(sixp_pkt_metadata_t); 
  }
  */
  if(type == SIXP_PKT_TYPE_REQUEST && (code.value == SIXP_PKT_CMD_ADD_UPLINKS || code.value == SIXP_PKT_CMD_ASK_ADV_LINK))
  {
     return sizeof(sixp_pkt_metadata_t); 
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_num_cells_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST &&
     (code.value == SIXP_PKT_CMD_ADD ||
      code.value == SIXP_PKT_CMD_DELETE ||
      code.value == SIXP_PKT_CMD_RELOCATE)) {
    return sizeof(sixp_pkt_metadata_t) + sizeof(sixp_pkt_cell_options_t);
  }
  /*
  if(type == SIXP_PKT_TYPE_REQUEST && code.value == SIXP_PKT_CMD_ADD_ADV)
  {
     return (sizeof(sixp_pkt_metadata_t) + sizeof(sixp_pkt_cell_options_t)); 
  }
  */
  if(type == SIXP_PKT_TYPE_REQUEST && (code.value == SIXP_PKT_CMD_ADD_UPLINKS  ||  code.value == SIXP_PKT_CMD_ASK_ADV_LINK))
  {
     return (sizeof(sixp_pkt_metadata_t) + sizeof(sixp_pkt_cell_options_t)); 
  }

  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_reserved_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST &&
     code.value == SIXP_PKT_CMD_LIST) {
    return sizeof(sixp_pkt_metadata_t) + sizeof(sixp_pkt_cell_options_t);
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_offset_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST &&
     code.value == SIXP_PKT_CMD_LIST) {
    return (sizeof(sixp_pkt_metadata_t) +
            sizeof(sixp_pkt_cell_options_t) +
            sizeof(sixp_pkt_reserved_t));
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_max_num_cells_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST &&
     code.value == SIXP_PKT_CMD_LIST) {
    return (sizeof(sixp_pkt_metadata_t) +
            sizeof(sixp_pkt_cell_options_t) +
            sizeof(sixp_pkt_reserved_t) +
            sizeof(sixp_pkt_offset_t));
  }
  return -1;
}

static int32_t
get_cell_list_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST && (code.value == SIXP_PKT_CMD_ADD ||
                                      // code.value == SIXP_PKT_CMD_ADD_ADV ||
                                       code.value == SIXP_PKT_CMD_DELETE )) {
    return (sizeof(sixp_pkt_metadata_t) +
            sizeof(sixp_pkt_cell_options_t) +
            sizeof(sixp_pkt_num_cells_t));
  } else if((type == SIXP_PKT_TYPE_RESPONSE ||
             type == SIXP_PKT_TYPE_CONFIRMATION) &&
            (code.value == SIXP_PKT_RC_SUCCESS ||
             code.value == SIXP_PKT_RC_EOL)) {
    return 0;
  }
  
  return -1;
}


static int32_t
dtsf_get_cell_list_offset_for_uplink(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
    if((type == SIXP_PKT_TYPE_RESPONSE ||
             type == SIXP_PKT_TYPE_CONFIRMATION) &&
            (code.value == SIXP_PKT_RC_SUCCESS ||
             code.value == SIXP_PKT_RC_EOL)) {
    return 4;
  }
  
  return -1;
}


static int32_t
dtsf_get_cell_list_offset_for_adv_link(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
    if((type == SIXP_PKT_TYPE_RESPONSE ||
             type == SIXP_PKT_TYPE_CONFIRMATION) &&
            (code.value == SIXP_PKT_RC_SUCCESS ||
             code.value == SIXP_PKT_RC_EOL)) {
    return 4;
  }
  
  return -1;
}
/*---------------------------------------------------------------------------*/

//dtsf/////////////////////////////////////////////////////////////////////////////////
static int32_t
get_channel_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST && (code.value == SIXP_PKT_CMD_ASK_CHANNEL)) 
  {
    
    return (sizeof(sixp_pkt_metadata_t));
  } 
  else if((type == SIXP_PKT_TYPE_RESPONSE || type == SIXP_PKT_TYPE_CONFIRMATION) && (code.value == SIXP_PKT_RC_SUCCESS))
  {
    return 0;
  } 
  return -1;
}

static int32_t
get_add_uplinks_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
    
  if(type == SIXP_PKT_TYPE_REQUEST && (code.value == SIXP_PKT_CMD_ADD_UPLINKS)) 
  {
    
    return (sizeof(sixp_pkt_metadata_t)+2);
  } 
  else if((type == SIXP_PKT_TYPE_RESPONSE || type == SIXP_PKT_TYPE_CONFIRMATION) && (code.value == SIXP_PKT_RC_SUCCESS))
  {
    return 0;
  } 
  return -1;
}



static int32_t
get_delete_downlinks_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
    
      if(type == SIXP_PKT_TYPE_REQUEST && (code.value == SIXP_PKT_CMD_DELETE_DOWNLINK)) 
  {
    
    return (sizeof(sixp_pkt_metadata_t)+2);
  } 
  else if((type == SIXP_PKT_TYPE_RESPONSE || type == SIXP_PKT_TYPE_CONFIRMATION) && (code.value == SIXP_PKT_RC_SUCCESS))
  {
    return 0;
  } 
  return -1;

}


static int32_t
get_delete_uplinks_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
    
      if(type == SIXP_PKT_TYPE_REQUEST && (code.value == SIXP_PKT_CMD_DELETE_UPLINK)) 
  {
    
    return (sizeof(sixp_pkt_metadata_t)+2);
  } 
  else if((type == SIXP_PKT_TYPE_RESPONSE || type == SIXP_PKT_TYPE_CONFIRMATION) && (code.value == SIXP_PKT_RC_SUCCESS))
  {
    return 0;
  } 
  return -1;

}



static int32_t
get_add_adv_link_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
    
  if(type == SIXP_PKT_TYPE_REQUEST && (code.value == SIXP_PKT_CMD_ASK_ADV_LINK)) 
  {
    
    return (sizeof(sixp_pkt_metadata_t)+2);
  } 
  else if((type == SIXP_PKT_TYPE_RESPONSE || type == SIXP_PKT_TYPE_CONFIRMATION) && (code.value == SIXP_PKT_RC_SUCCESS))
  {
    return 0;
  } 
  return -1;
}
///////////////////////////////////////////////
/*---------------------------------------------------------------------------*/
static int32_t
get_rel_cell_list_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST && code.value == SIXP_PKT_CMD_RELOCATE) {
    return (sizeof(sixp_pkt_metadata_t) +
            sizeof(sixp_pkt_cell_options_t) +
            sizeof(sixp_pkt_num_cells_t));
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_total_num_cells_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_RESPONSE && code.value == SIXP_PKT_RC_SUCCESS) {
    return 0;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
get_payload_offset(sixp_pkt_type_t type, sixp_pkt_code_t code)
{
  if(type == SIXP_PKT_TYPE_REQUEST && code.value == SIXP_PKT_CMD_SIGNAL) {
    return sizeof(sixp_pkt_metadata_t);
  } else if((type == SIXP_PKT_TYPE_RESPONSE ||
             type == SIXP_PKT_TYPE_CONFIRMATION) &&
            code.value == SIXP_PKT_RC_SUCCESS) {
    return 0;
  } 
  printf("returnning -1\n");
  return -1;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_metadata(sixp_pkt_type_t type, sixp_pkt_code_t code,
                      sixp_pkt_metadata_t metadata,
                      uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set metadata; body is null\n");
    return -1;
  }

  if((offset = get_metadata_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set metadata [type=%u, code=%u], invalid type\n",
            type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(metadata))) {
    LOG_ERR("6P-pkt: cannot set metadata, body is too short [body_len=%u]\n",
            body_len);
    return -1;
  }

  /*
   * Copy the content into the Metadata field as it is since 6P has no idea
   * about the internal structure of the field.
   */
  memcpy(body + offset, &metadata, sizeof(metadata));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_metadata(sixp_pkt_type_t type, sixp_pkt_code_t code,
                      sixp_pkt_metadata_t *metadata,
                      const uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(metadata == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get metadata; invalid argument\n");
    return -1;
  }

  if((offset = get_metadata_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot get metadata [type=%u, code=%u], invalid type\n",
            type, code.value);
    return -1;
  }

  if(body_len < offset + sizeof(*metadata)) {
    LOG_ERR("6P-pkt: cannot get metadata [type=%u, code=%u], ",
            type, code.value);
    LOG_ERR_("body is too short\n");
    return -1;
  }

  /*
   * Copy the content in the Metadata field as it is since 6P has no idea about
   * the internal structure of the field.
   */
  memcpy(metadata, body + offset, sizeof(*metadata));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_cell_options(sixp_pkt_type_t type, sixp_pkt_code_t code,
                          sixp_pkt_cell_options_t cell_options,
                          uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set cell_options; body is null\n");
    return -1;
  }

  if((offset = get_cell_options_offset(type, code)) < 0) {
    printf("6P-pkt: cannot set cell_options [type=%u, code=%u], ",
            type, code.value);
    printf("invalid type\n");
    return -1;
  }

  if(body_len < (offset + sizeof(cell_options))) {
    printf("6P-pkt: cannot set cell_options, ");
    printf("body is too short [body_len=%u]\n", body_len);
    return -1;
  }
 // printf("omid: cell option offset %d\n",(int)offset);
  /* The CellOption field is an 8-bit bitfield */
  memcpy(body + offset, &cell_options, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_cell_options(sixp_pkt_type_t type, sixp_pkt_code_t code,
                          sixp_pkt_cell_options_t *cell_options,
                          const uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(cell_options == NULL || body == NULL) {
    printf("6P-pkt: cannot get cell_options; invalid argument\n");
    return -1;
  }

  if((offset = get_cell_options_offset(type, code)) < 0) {
    printf("6P-pkt: cannot get cell_options [type=%u, code=%u]",
            type, code.value);
    printf("invalid type\n");
    return -1;
  }

  if(body_len < (offset + sizeof(*cell_options))) {
    printf("6P-pkt: cannot get cell_options\n");
    LOG_ERR_("body is too short [body_len=%u]\n", body_len);
    return -1;
  }

  /* The CellOption field is an 8-bit bitfield */
  memcpy(cell_options, body + offset, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_num_cells(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       sixp_pkt_num_cells_t num_cells,
                       uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set num_cells; body is null\n");
    return -1;
  }

  if((offset = get_num_cells_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set num_cells; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have NumCells\n",
             type, code.value);
    return -1;
  }
 //  printf("omid: num of cell offset %d\n",(int)offset);
  memcpy(body + offset, &num_cells, sizeof(uint8_t));

  return 0;
}


///omid-dtsf///////////////////////////////////////////////////////////////////////////
/*
int
sixp_pkt_set_frequency_children(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       uint8_t frequency_channel,
                       uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set num_cells; body is null\n");
    return -1;
  }

  if((offset = get_num_cells_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set num_cells; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have NumCells\n",
             type, code.value);
    return -1;
  }

  memcpy(body + offset, &num_cells, sizeof(uint8_t));

  return 0;
}
 */
//////////////////////////////////////////////////////////////////////////////////////



/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_num_cells(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       sixp_pkt_num_cells_t *num_cells,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(num_cells == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get num_cells; invalid argument\n");
    return -1;
  }

  if((offset = get_num_cells_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot get num_cells; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have NumCells\n",
             type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(*num_cells))) {
    LOG_ERR("6P-pkt: cannot get num_cells; body is too short\n");
    return -1;
  }

  /* NumCells is an 8-bit unsigned integer */
  memcpy(num_cells, body + offset, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_reserved(sixp_pkt_type_t type, sixp_pkt_code_t code,
                      sixp_pkt_reserved_t reserved,
                      uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set reserved; body is null\n");
    return -1;
  }

  if((offset = get_reserved_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set reserved; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have Reserved\n",
             type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(reserved))) {
    LOG_ERR("6P-pkt: cannot set reserved; body is too short\n");
    return -1;
  }

  /* The Reserved field is an 8-bit field */
  memcpy(body + offset, &reserved, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_reserved(sixp_pkt_type_t type, sixp_pkt_code_t code,
                      sixp_pkt_reserved_t *reserved,
                      const uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(reserved == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get reserved; invalid argument\n");
    return -1;
  }

  if((offset = get_reserved_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot get reserved; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have Reserved\n",
             type, code.value);
    return -1;
  }

  /* The Reserved field is an 8-bit field */
  memcpy(reserved, body + offset, sizeof(uint8_t));

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_offset(sixp_pkt_type_t type, sixp_pkt_code_t code,
                    sixp_pkt_offset_t cell_offset,
                    uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set offset; invalid argument\n");
    return -1;
  }

  if((offset = get_offset_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set offset; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have Offset\n",
             type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(cell_offset))) {
    LOG_ERR("6P-pkt: cannot set offset; body is too short\n");
    return -1;
  }

  /*
   * The (Cell)Offset field is 16-bit long; treat it as a little-endian value of
   * unsigned integer following IEEE 802.15.4-2015.
   */
  (body + offset)[0] = *((uint16_t *)&cell_offset) & 0xff;
  (body + offset)[1] = (*((uint16_t *)&cell_offset) >> 8) & 0xff;

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_offset(sixp_pkt_type_t type, sixp_pkt_code_t code,
                    sixp_pkt_offset_t *cell_offset,
                    const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  const uint8_t *p;

  if(cell_offset == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get offset; invalid argument\n");
    return -1;
  }

  if((offset = get_offset_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot get offset; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have Offset\n",
             type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(*cell_offset))) {
    LOG_ERR("6P-pkt: cannot get offset; body is too short\n");
    return -1;
  }

  /*
   * The (Cell)Offset field is 16-bit long; treat it as a little-endian value of
   * unsigned integer following IEEE 802.15.4-2015.
   */
  p = body + offset;
  *((uint16_t *)cell_offset) = p[0] + (p[1] << 8);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_max_num_cells(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           sixp_pkt_max_num_cells_t max_num_cells,
                           uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set max_num_cells; invalid argument\n");
    return -1;
  }

  if((offset = get_max_num_cells_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set max_num_cells; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have MaxNumCells\n",
             type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(max_num_cells))) {
    LOG_ERR("6P-pkt: cannot set max_num_cells; body is too short\n");
    return -1;
  }

  /*
   * The MaxNumCells field is 16-bit long; treat it as a little-endian value of
   * unsigned integer following IEEE 802.15.4-2015.
   */
  (body + offset)[0] = *((uint16_t *)&max_num_cells) & 0xff;
  (body + offset)[1] = (*((uint16_t *)&max_num_cells) >> 8) & 0xff;

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_max_num_cells(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           sixp_pkt_max_num_cells_t *max_num_cells,
                           const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  const uint8_t *p;

  if(max_num_cells == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get max_num_cells; invalid argument\n");
    return -1;
  }

  if((offset = get_max_num_cells_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot get max_num_cells; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have MaxNumCells\n",
             type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(*max_num_cells))) {
    LOG_ERR("6P-pkt: cannot get max_num_cells; body is too short\n");
    return -1;
  }

  /*
   * The MaxNumCells field is 16-bit long; treat it as a little-endian value of
   * unsigned integer following IEEE 802.15.4-2015.
   */
  p = body + offset;
  *((uint16_t *)max_num_cells) = p[0] + (p[1] << 8);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_cell_list(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t *cell_list,
                       uint16_t cell_list_len,
                       uint16_t cell_offset,
                       uint8_t *body, uint16_t body_len)
{
  int32_t offset;
//printf("omid:body_len=%d \n",(int)body_len);
  if(cell_list == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot set cell_list; invalid argument\n");
    return -1;
  }

  if((offset = get_cell_list_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set cell_list; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have CellList\n",
             type, code.value);
    return -1;
  }

  offset += cell_offset;

  if(body_len < (offset + cell_list_len)) {
 
    LOG_ERR("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if((cell_list_len % sizeof(sixp_pkt_cell_t)) != 0) {
       //  printf("omid:body_len=%d  offset=%d cell_list_len=%d\n",(int)body_len,(int)offset,(int)cell_list_len);
    LOG_ERR("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }
  
  memcpy(body + offset, cell_list, cell_list_len);

  return 0;
}


int sixp_pkt_set_cell_list_for_uplink(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           const uint8_t *cell_list,
                           uint16_t cell_list_len,
                           uint16_t cell_offset,
                           uint8_t *body, uint16_t body_len)
{
int16_t offset;
  //printf("omid:body_len=%d \n",(int)body_len);
  if(cell_list == NULL || body == NULL) {
    printf("6P-pkt: cannot set cell_list; invalid argument\n");
    return -1;
  }

  if((offset = dtsf_get_cell_list_offset_for_uplink(type, code)) < 0) {
    printf("6P-pkt: cannot set cell_list; ");
    printf("packet13 [type=%u, code=%u] won't have CellList\n",
             type, code.value);
    return -1;
  }

  offset += cell_offset*4;

  if(body_len < (offset + cell_list_len*4)) {
 
    printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if((cell_list_len % sizeof(sixp_pkt_cell_t)) != 0) {
       //  printf("omid:body_len=%d  offset=%d cell_list_len=%d\n",(int)body_len,(int)offset,(int)cell_list_len);
    printf("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }
  
  memcpy(body + offset, cell_list, cell_list_len*4);

  return 0;
}


int sixp_pkt_set_cell_list_for_downlink(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           const uint8_t *cell_list,
                           uint16_t cell_list_len,
                           uint16_t cell_offset,
                           uint8_t *body, uint16_t body_len)
{
int16_t offset;
  //printf("omid:body_len=%d \n",(int)body_len);
  if(cell_list == NULL || body == NULL) {
    printf("6P-pkt: cannot set cell_list; invalid argument\n");
    return -1;
  }

offset=8;

  offset += cell_offset*4;

  if(body_len < (offset + cell_list_len*4)) {
 
    printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if((cell_list_len % sizeof(sixp_pkt_cell_t)) != 0) {
       //  printf("omid:body_len=%d  offset=%d cell_list_len=%d\n",(int)body_len,(int)offset,(int)cell_list_len);
    printf("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }
  
  memcpy(body + offset, cell_list, cell_list_len*4);

  return 0;
}



int sixp_pkt_set_cell_list_for_delete_downlink(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           const uint8_t *cell_list,
                           uint16_t cell_list_len,
                           uint16_t cell_offset,
                           uint8_t *body, uint16_t body_len)
{
    int16_t offset;
  
  if(cell_list == NULL || body == NULL) 
  {
        printf("6P-pkt: cannot set cell_list; invalid argument\n");
        return -1;
  }

  offset=8;

  offset += cell_offset*4;

  if(body_len < (offset + cell_list_len*4)) {
 
    printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if((cell_list_len % sizeof(sixp_pkt_cell_t)) != 0) {
       //  printf("omid:body_len=%d  offset=%d cell_list_len=%d\n",(int)body_len,(int)offset,(int)cell_list_len);
    printf("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }
  
  memcpy(body + offset, cell_list, cell_list_len*4);

  return 0;
}


int sixp_pkt_set_cell_list_for_delete_uplink(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           const uint8_t *cell_list,
                           uint16_t cell_list_len,
                           uint16_t cell_offset,
                           uint8_t *body, uint16_t body_len)
{
    int16_t offset;
  
  if(cell_list == NULL || body == NULL) 
  {
        printf("6P-pkt: cannot set cell_list; invalid argument\n");
        return -1;
  }

  offset=8;

  offset += cell_offset*4;

  if(body_len < (offset + cell_list_len*4)) {
 
    printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if((cell_list_len % sizeof(sixp_pkt_cell_t)) != 0) {
       //  printf("omid:body_len=%d  offset=%d cell_list_len=%d\n",(int)body_len,(int)offset,(int)cell_list_len);
    printf("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }
  
  memcpy(body + offset, cell_list, cell_list_len*4);

  return 0;
}



int sixp_pkt_set_cell_list_for_adv_link(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           const uint8_t *cell_list,
                           uint16_t cell_list_len,
                           uint16_t cell_offset,
                           uint8_t *body, uint16_t body_len)
{
int16_t offset;
//printf("omid:body_len=%d \n",(int)body_len);
  if(cell_list == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot set cell_list; invalid argument\n");
    return -1;
  }

  if((offset = dtsf_get_cell_list_offset_for_adv_link(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set cell_list; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have CellList\n",
             type, code.value);
    return -1;
  }

  offset += cell_offset*4;

  if(body_len < (offset + cell_list_len*4)) {
 
    LOG_ERR("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if((cell_list_len % sizeof(sixp_pkt_cell_t)) != 0) {
       //  printf("omid:body_len=%d  offset=%d cell_list_len=%d\n",(int)body_len,(int)offset,(int)cell_list_len);
    LOG_ERR("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }
  
  memcpy(body + offset, cell_list, cell_list_len*4);

  return 0;
}
//---------------------------------------------------------------------------------
//dtsf/////////////////////////////////////////////////////////////////////////////
int
sixp_pkt_set_channel(sixp_pkt_type_t type, sixp_pkt_code_t code, uint16_t channel,
                       uint8_t *body, uint16_t body_len)
{
  int32_t offset;
//printf("omid:body_len=%d \n",(int)body_len);
  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set channel; invalid argument\n");
    return -1;
  }

  if((offset = get_channel_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set channel; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have channel\n",
             type, code.value);
    return -1;
  }

 

  if(body_len < (offset + 2)) {
 
    LOG_ERR("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } 
  
  memcpy(body + offset, &channel, 2);

  return 0;
}

uint16_t
sixp_pkt_set_request_add_uplink(sixp_pkt_type_t type, sixp_pkt_code_t code, uint32_t number_of_links,
                              uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  //printf("omid:send uplink request %d\n",(int)body_len);
  if(body == NULL) {
    printf("6P-pkt: cannot set request_uplinks; invalid argument\n");
    return -1;
  }

  if((offset = get_add_uplinks_offset(type, code)) < 0) {
    printf("6P-pkt: cannot set channel; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have channel\n",
             type, code.value);
    return -1;
  }

  //printf("end2461 %d\n",(int)offset);

  if(body_len < (offset + 4)) {
 
    printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } 
  
  memcpy(body + offset, &number_of_links, sizeof(uint32_t));
  /*         int i=0;
  for(i=0;i<8;i++)
  {
      printf("%02x ",body[i]);
  }
  printf("end2462\n");
*/  return 0;
}

uint16_t
sixp_pkt_set_request_add_downlink(sixp_pkt_type_t type, sixp_pkt_code_t code, uint32_t number_of_links,
                              uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  //printf("omid:send uplink request %d\n",(int)body_len);
  if(body == NULL) {
    printf("6P-pkt: cannot set request_uplinks; invalid argument\n");
    return -1;
  }

  offset = 4;

  //printf("end2461 %d\n",(int)offset);

  if(body_len < (offset + 4)) {
 
    printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } 
  
  memcpy(body + offset, &number_of_links, sizeof(uint32_t));
  /*         int i=0;
  for(i=0;i<8;i++)
  {
      printf("%02x ",body[i]);
  }
  printf("end2462\n");
*/  return 0;
}

uint32_t
sixp_pkt_get_request_add_uplink(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
//printf("omid:body_len=%d \n",(int)body_len);
  if(body == NULL) {
    printf("6P-pkt: cannot set request_uplinks; invalid argument\n");
    return -1;
  }
  /*
  int i=0;
  for(i=0;i<8;i++)
  {
      printf("%02x ",body[i]);
  }
  printf("end222\n");
*/
  if((offset = get_add_uplinks_offset(type, code)) < 0) 
  {
    printf("6P-pkt: cannot set channel; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have channel\n",
             type, code.value);
    return -1;
  }
 
   //printf("offset=%d\n",(int)offset);
  
  if(body_len < (offset+4)) 
  {
    printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } 
  
  uint32_t number_of_links;
  memcpy(&number_of_links, body+offset, 4);

  return number_of_links;
}


uint32_t
sixp_pkt_get_request_add_downlink(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
//printf("omid:body_len=%d \n",(int)body_len);
  if(body == NULL) {
    printf("6P-pkt: cannot set request_uplinks; invalid argument\n");
    return -1;
  }
  /*
  int i=0;
  for(i=0;i<8;i++)
  {
      printf("%02x ",body[i]);
  }
  printf("end222\n");
*/
offset=4;
 
   //printf("offset=%d\n",(int)offset);
  
  if(body_len < (offset+4)) 
  {
    printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } 
  
  uint32_t number_of_links;
  memcpy(&number_of_links, body+offset, 4);

  return number_of_links;
}


uint16_t
sixp_pkt_set_request_delete_downlinks(sixp_pkt_type_t type, sixp_pkt_code_t code, uint32_t number_of_links,
                              uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  //printf("omid:send uplink request %d\n",(int)body_len);
  if(body == NULL) {
    printf("6P-pkt: cannot set request_uplinks; invalid argument\n");
    return -1;
  }

  if((offset = get_delete_downlinks_offset(type, code)) < 0) {
    printf("6P-pkt: cannot set channel; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have channel\n",
             type, code.value);
    return -1;
  }

  //printf("end2461 %d\n",(int)offset);

  if(body_len < (offset + 4)) {
 
    printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } 
  
  memcpy(body + offset, &number_of_links, sizeof(uint32_t));
  /*         int i=0;
  for(i=0;i<8;i++)
  {
      printf("%02x ",body[i]);
  }
  printf("end2462\n");
*/  return 0;
}


uint16_t
sixp_pkt_set_request_delete_uplinks(sixp_pkt_type_t type, sixp_pkt_code_t code, uint32_t number_of_links,
                              uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  //printf("omid:send uplink request %d\n",(int)body_len);
  if(body == NULL) {
    printf("6P-pkt: cannot set request_uplinks; invalid argument\n");
    return -1;
  }

  if((offset = get_delete_uplinks_offset(type, code)) < 0) {
    printf("6P-pkt: cannot set channel; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have channel\n",
             type, code.value);
    return -1;
  }

  //printf("end2461 %d\n",(int)offset);

  if(body_len < (offset + 4)) {
 
    printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } 
  
  memcpy(body + offset, &number_of_links, sizeof(uint32_t));
  /*         int i=0;
  for(i=0;i<8;i++)
  {
      printf("%02x ",body[i]);
  }
  printf("end2462\n");
*/  return 0;
}



uint16_t
sixp_pkt_set_requested_adv_link(sixp_pkt_type_t type, sixp_pkt_code_t code, uint32_t number_of_links,
                              uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  //printf("omid:send adv request %d\n",(int)body_len);
  if(body == NULL) {
    printf("6P-pkt: cannot set request_advlink; invalid argument\n");
    return -1;
  }

  if((offset = get_add_adv_link_offset(type, code)) < 0) {
    printf("6P-pkt: cannot set channel; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have channel\n",
             type, code.value);
    return -1;
  }

  //printf("end2461 %d\n",(int)offset);

  if(body_len < (offset + 4)) {
 
    printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } 
  
  memcpy(body + offset, &number_of_links, sizeof(uint32_t));
       /*    int i=0;
  for(i=0;i<8;i++)
  {
      printf("%02x ",body[i]);
  }
  printf("end2462\n");
  */return 0;
}



uint32_t
sixp_pkt_get_requested_adv_link(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
//printf("omid:body_len=%d \n",(int)body_len);
  if(body == NULL) {
    printf("6P-pkt: cannot set request_adv_link; invalid argument\n");
    return -1;
  }
  
 /* int i=0;
  for(i=0;i<8;i++)
  {
      printf("%02x ",body[i]);
  }
  printf("end222\n");
*/
  if((offset = get_add_adv_link_offset(type, code)) < 0) 
  {
    printf("6P-pkt: cannot set channel; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have channel\n",
             type, code.value);
    return -1;
  }
 
  // printf("offset=%d\n",(int)offset);
  
  if(body_len < (offset+4)) 
  {
    printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } 
  
  uint32_t number_of_links;
  memcpy(&number_of_links, body+offset, 4);

  return number_of_links;
}


int
sixp_pkt_set_response_uplinks(sixp_pkt_type_t type, sixp_pkt_code_t code, uint32_t number_of_links,
                              uint8_t *body, uint16_t body_len)
{
   int32_t offset;
//printf("omid:body_len=%d \n",(int)body_len);
  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set request_uplinks; invalid argument\n");
    return -1;
  }

  if((offset = get_add_uplinks_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set channel; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have channel\n",
             type, code.value);
    return -1;
  }

 

  if(body_len < (offset + 4)) {
 
    LOG_ERR("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } 
  
  memcpy(body + offset, &number_of_links, 4);

  return 0;
}




int
sixp_pkt_set_response_adv_link(sixp_pkt_type_t type, sixp_pkt_code_t code, uint32_t number_of_links,
                              uint8_t *body, uint16_t body_len)
{
   int32_t offset;
//printf("omid:body_len=%d \n",(int)body_len);
  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set request_uplinks; invalid argument\n");
    return -1;
  }

  if((offset = get_add_adv_link_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set channel; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have channel\n",
             type, code.value);
    return -1;
  }

 

  if(body_len < (offset + 4)) {
 
    LOG_ERR("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } 
  
  memcpy(body + offset, &number_of_links, 4);

  return 0;
}



uint16_t
sixp_pkt_get_channel(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  //printf("omid:get channel\n");
  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set channel; invalid argument\n");
    return -1;
  }

  if((offset = get_channel_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set channel; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have channel\n",
             type, code.value);
    printf("channel 1\n");
    return -1;
  }

 

  if(body_len < (offset + 2)) {
    printf("channel 2\n");
    LOG_ERR("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } 
  uint16_t channel;
  memcpy(&channel, body+offset, 2);
  //printf("channel=%d\n",channel);
  return channel;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_cell_list(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t **cell_list,
                       sixp_pkt_offset_t *cell_list_len,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
//printf("omid1:body_len=%d\n",(int)body_len);
  if(cell_list_len == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get cell_list\n");
        printf("6P-pkt: cannot get cell_list\n");

    return -1;
  }

  if((offset = get_cell_list_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot get cell_list; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have CellList\n",
             type, code.value);
        printf("packet11 [type=%u, code=%u] won't have CellList\n",
             type, code.value);
    return -1;
  }

  if(body_len < offset) {
    LOG_ERR("6P-pkt: cannot set cell_list; body is too short\n");
       printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if(((body_len - offset) % sizeof(sixp_pkt_cell_t)) != 0) {
    LOG_ERR("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
   // printf("omid1:body_len=%d  offset=%d cell_list_len=%d\n",(int)body_len,(int)offset,(int)cell_list_len);
       printf("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }

  if(cell_list != NULL) {
    *cell_list = body + offset;
  }

  *cell_list_len = body_len - offset;

  return 0;
}


int
sixp_pkt_get_cell_list_for_uplink(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t **cell_list,
                       sixp_pkt_offset_t *cell_list_len,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
//printf("omid1:body_len=%d\n",(int)body_len);
  if(cell_list_len == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get cell_list\n");
        printf("6P-pkt: cannot get cell_list\n");

    return -1;
  }

  if((offset = dtsf_get_cell_list_offset_for_uplink(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot get cell_list; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have CellList\n",
             type, code.value);
        printf("packet14 [type=%u, code=%u] won't have CellList\n",
             type, code.value);
    return -1;
  }

  if(body_len < offset) {
    LOG_ERR("6P-pkt: cannot set cell_list; body is too short\n");
       printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if(((body_len - offset) % sizeof(sixp_pkt_cell_t)) != 0) {
    LOG_ERR("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
   // printf("omid1:body_len=%d  offset=%d cell_list_len=%d\n",(int)body_len,(int)offset,(int)cell_list_len);
       printf("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }

  if(cell_list != NULL) {
    *cell_list = body + offset;
  }

  *cell_list_len = body_len - offset;
  
  //printf("766 len=%d offset=%d\n",(int)*cell_list_len, (int)offset);

  return 0;
}


int
sixp_pkt_get_cell_list_for_downlink(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t **cell_list,
                       sixp_pkt_offset_t *cell_list_len,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
//printf("omid1:body_len=%d\n",(int)body_len);
  if(cell_list_len == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get cell_list\n");
        printf("6P-pkt: cannot get cell_list\n");

    return -1;
  }

  offset = 8;

  if(body_len < offset) {
    LOG_ERR("6P-pkt: cannot set cell_list; body is too short\n");
       printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if(((body_len - offset) % sizeof(sixp_pkt_cell_t)) != 0) {
    LOG_ERR("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
   // printf("omid1:body_len=%d  offset=%d cell_list_len=%d\n",(int)body_len,(int)offset,(int)cell_list_len);
       printf("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }

  if(cell_list != NULL) {
    *cell_list = body + offset;
  }

  *cell_list_len = body_len - offset;
  
  //printf("766 len=%d offset=%d\n",(int)*cell_list_len, (int)offset);

  return 0;
}


int
sixp_pkt_get_cell_list_for_delete_downlink(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t **cell_list,
                       sixp_pkt_offset_t *cell_list_len,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
//printf("omid1:body_len=%d\n",(int)body_len);
  if(cell_list_len == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get cell_list\n");
        printf("6P-pkt: cannot get cell_list\n");

    return -1;
  }

  offset = 8;

  if(body_len < offset) {
    LOG_ERR("6P-pkt: cannot set cell_list; body is too short\n");
       printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if(((body_len - offset) % sizeof(sixp_pkt_cell_t)) != 0) {
    LOG_ERR("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
   // printf("omid1:body_len=%d  offset=%d cell_list_len=%d\n",(int)body_len,(int)offset,(int)cell_list_len);
       printf("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }

  if(cell_list != NULL) {
    *cell_list = body + offset;
  }

  *cell_list_len = body_len - offset;
  
  //printf("766 len=%d offset=%d\n",(int)*cell_list_len, (int)offset);

  return 0;
}


int
sixp_pkt_get_cell_list_for_delete_uplink(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t **cell_list,
                       sixp_pkt_offset_t *cell_list_len,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
//printf("omid1:body_len=%d\n",(int)body_len);
  if(cell_list_len == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get cell_list\n");
        printf("6P-pkt: cannot get cell_list\n");

    return -1;
  }

  offset = 8;

  if(body_len < offset) {
    LOG_ERR("6P-pkt: cannot set cell_list; body is too short\n");
       printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if(((body_len - offset) % sizeof(sixp_pkt_cell_t)) != 0) {
    LOG_ERR("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
   // printf("omid1:body_len=%d  offset=%d cell_list_len=%d\n",(int)body_len,(int)offset,(int)cell_list_len);
       printf("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }

  if(cell_list != NULL) {
    *cell_list = body + offset;
  }

  *cell_list_len = body_len - offset;
  
  //printf("766 len=%d offset=%d\n",(int)*cell_list_len, (int)offset);

  return 0;
}

int
sixp_pkt_get_cell_list_for_adv_link(sixp_pkt_type_t type, sixp_pkt_code_t code,
                       const uint8_t **cell_list,
                       sixp_pkt_offset_t *cell_list_len,
                       const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
//printf("omid1:body_len=%d\n",(int)body_len);
  if(cell_list_len == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get cell_list\n");
        printf("6P-pkt: cannot get cell_list\n");

    return -1;
  }

  if((offset = dtsf_get_cell_list_offset_for_adv_link(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot get cell_list; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have CellList\n",
             type, code.value);
        printf("packet15 [type=%u, code=%u] won't have CellList\n",
             type, code.value);
    return -1;
  }

  if(body_len < offset) {
    LOG_ERR("6P-pkt: cannot set cell_list; body is too short\n");
       printf("6P-pkt: cannot set cell_list; body is too short\n");
    return -1;
  } else if(((body_len - offset) % sizeof(sixp_pkt_cell_t)) != 0) {
    LOG_ERR("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
   // printf("omid1:body_len=%d  offset=%d cell_list_len=%d\n",(int)body_len,(int)offset,(int)cell_list_len);
       printf("6P-pkt: cannot set cell_list; invalid {body, cell_list}_len\n");
    return -1;
  }

  if(cell_list != NULL) {
    *cell_list = body + offset;
  }

  *cell_list_len = body_len - offset;
  
  //printf("766 len=%d offset=%d\n",(int)*cell_list_len, (int)offset);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_rel_cell_list(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           const uint8_t *rel_cell_list,
                           uint16_t rel_cell_list_len,
                           uint16_t cell_offset,
                           uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  sixp_pkt_num_cells_t num_cells;

  if(rel_cell_list == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot set rel_cell_list; invalid argument\n");
    return -1;
  }

  if(sixp_pkt_get_num_cells(type, code, &num_cells, body, body_len) < 0) {
    LOG_ERR("6P-pkt: cannot set rel_cell_list; no NumCells field\n");
    return -1;
  }

  if((offset = get_rel_cell_list_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set rel_cell_list; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have RelCellList\n",
             type, code.value);
    return -1;
  }

  offset += cell_offset;

  if(body_len < (offset + rel_cell_list_len)) {
    LOG_ERR("6P-pkt: cannot set rel_cell_list; body is too short\n");
    return -1;
  } else if((offset + rel_cell_list_len) >
            (offset + num_cells * sizeof(sixp_pkt_cell_t))) {
    LOG_ERR("6P-pkt: cannot set rel_cell_list; RelCellList is too long\n");
    return -1;
  } else if((rel_cell_list_len % sizeof(sixp_pkt_cell_t)) != 0) {
    LOG_ERR("6P-pkt: cannot set rel_cell_list; invalid body_len\n");
    return -1;
  }

  memcpy(body + offset, rel_cell_list, rel_cell_list_len);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_rel_cell_list(sixp_pkt_type_t type, sixp_pkt_code_t code,
                           const uint8_t **rel_cell_list,
                           sixp_pkt_offset_t *rel_cell_list_len,
                           const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  sixp_pkt_num_cells_t num_cells;

  if(rel_cell_list_len == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get rel_cell_list; invalid argument\n");
    return -1;
  }

  if(sixp_pkt_get_num_cells(type, code, &num_cells, body, body_len) < 0) {
    LOG_ERR("6P-pkt: cannot get rel_cell_list; no NumCells field\n");
    return -1;
  }

  if((offset = get_rel_cell_list_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot get rel_cell_list; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have RelCellList\n",
             type, code.value);
    return -1;
  }

  if(body_len < (offset + (num_cells * sizeof(sixp_pkt_cell_t)))) {
    LOG_ERR("6P-pkt: cannot set rel_cell_list; body is too short\n");
    return -1;
  } else if(((body_len - offset) % sizeof(sixp_pkt_cell_t)) != 0) {
    LOG_ERR("6P-pkt: cannot set rel_cell_list; invalid body_len\n");
    return -1;
  }

  if(rel_cell_list != NULL) {
    *rel_cell_list = body + offset;
  }

  *rel_cell_list_len = num_cells * sizeof(sixp_pkt_cell_t);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_cand_cell_list(sixp_pkt_type_t type, sixp_pkt_code_t code,
                            const uint8_t *cand_cell_list,
                            uint16_t cand_cell_list_len,
                            uint16_t cell_offset,
                            uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  sixp_pkt_num_cells_t num_cells;

  if(cand_cell_list == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot set cand_cell_list; invalid argument\n");
    return -1;
  }

  if(sixp_pkt_get_num_cells(type, code, &num_cells, body, body_len) < 0) {
    LOG_ERR("6P-pkt: cannot set cand_cell_list; no NumCells field\n");
    return -1;
  }

  if((offset = get_rel_cell_list_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set cand_cell_list; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have RelCellList\n",
             type, code.value);
    return -1;
  }

  offset += cell_offset + num_cells * sizeof(sixp_pkt_cell_t);

  if(body_len < (offset + cand_cell_list_len)) {
    LOG_ERR("6P-pkt: cannot set cand_cell_list; body is too short\n");
    return -1;
  } else if((cand_cell_list_len % sizeof(sixp_pkt_cell_t)) != 0) {
    LOG_ERR("6P-pkt: cannot set cand_cell_list; invalid body_len\n");
    return -1;
  }

  memcpy(body + offset, cand_cell_list, cand_cell_list_len);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_cand_cell_list(sixp_pkt_type_t type, sixp_pkt_code_t code,
                            const uint8_t **cand_cell_list,
                            sixp_pkt_offset_t *cand_cell_list_len,
                            const uint8_t *body, uint16_t body_len)
{
  int32_t offset;
  sixp_pkt_num_cells_t num_cells;

  if(cand_cell_list_len == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get cand_cell_list; invalid argument\n");
    return -1;
  }

  if(sixp_pkt_get_num_cells(type, code, &num_cells, body, body_len) < 0) {
    LOG_ERR("6P-pkt: cannot get cand_cell_list; no NumCells field\n");
    return -1;
  }

  if((offset = get_rel_cell_list_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot get cand_cell_list; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have RelCellList\n",
             type, code.value);
    return -1;
  }

  offset += num_cells * sizeof(sixp_pkt_cell_t);

  if(body_len < offset) {
    LOG_ERR("6P-pkt: cannot set cand_cell_list; body is too short\n");
    return -1;
  } else if(((body_len - offset) % sizeof(sixp_pkt_cell_t)) != 0) {
    LOG_ERR("6P-pkt: cannot set cand_cell_list; invalid body_len\n");
    return -1;
  }

  if(cand_cell_list != NULL) {
    *cand_cell_list = body + offset;
  }

  *cand_cell_list_len = body_len - offset;

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_total_num_cells(sixp_pkt_type_t type, sixp_pkt_code_t code,
                             sixp_pkt_total_num_cells_t total_num_cells,
                             uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set num_cells; body is null\n");
    return -1;
  }

  if((offset = get_total_num_cells_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set total_num_cells; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have TotalNumCells\n",
             type, code.value);
    return -1;
  }

  /*
   * TotalNumCells for 6P Response is a 16-bit unsigned integer, little-endian.
   */
  body[offset] = (uint8_t)(total_num_cells & 0xff);
  body[offset + 1] = (uint8_t)(total_num_cells >> 8);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_total_num_cells(sixp_pkt_type_t type, sixp_pkt_code_t code,
                             sixp_pkt_total_num_cells_t *total_num_cells,
                             const uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(total_num_cells == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get num_cells; invalid argument\n");
    return -1;
  }

  if((offset = get_total_num_cells_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot get num_cells; ");
    LOG_ERR_("packet [type=%u, code=%u] won't have TotalNumCells\n",
             type, code.value);
    return -1;
  }

  if(body_len < (offset + sizeof(sixp_pkt_total_num_cells_t))) {
    LOG_ERR("6P-pkt: cannot get num_cells; body is too short\n");
    return -1;
  }

  /* TotalNumCells is a 16-bit unsigned integer, little-endian. */
  *total_num_cells = body[0];
  *total_num_cells += ((uint16_t)body[1]) << 8;

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_set_payload(sixp_pkt_type_t type, sixp_pkt_code_t code,
                     const uint8_t *payload, uint16_t payload_len,
                     uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(body == NULL) {
    LOG_ERR("6P-pkt: cannot set metadata; body is null\n");
    printf("6P-pkt: cannot set metadata; body is null\n");
    return -1;
  }

  if((offset = get_payload_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot set payload [type=%u, code=%u], invalid type\n",type, code.value);
    printf("6P-pkt: cannot set payload [type=%u, code=%u], invalid type\n",type, code.value);

    return -1;
  }

  if(body_len < (offset + payload_len)) {
    LOG_ERR("6P-pkt: cannot set payload, body is too short [body_len=%u]\n", body_len);
    printf("6P-pkt: cannot set payload, body is too short [body_len=%u]\n", body_len);
    return -1;
  }

  /*
   * Copy the content into the Payload field as it is since 6P has no idea
   * about the internal structure of the field.
   */
  printf("omid:set payload=%d  len=%d \n",(int)offset,(int)body_len);
  memcpy(body + offset, payload, payload_len);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_get_payload(sixp_pkt_type_t type, sixp_pkt_code_t code,
                     uint8_t *buf, uint16_t buf_len,
                     const uint8_t *body, uint16_t body_len)
{
  int32_t offset;

  if(buf == NULL || body == NULL) {
    LOG_ERR("6P-pkt: cannot get payload; invalid argument\n");
    printf("6P-pkt: cannot get payload; invalid argument\n");
    return -1;
  }

  if((offset = get_payload_offset(type, code)) < 0) {
    LOG_ERR("6P-pkt: cannot get payload [type=%u, code=%u], invalid type\n",
            type, code.value);
    printf("6P-pkt: cannot get payload [type=%u, code=%u], invalid type\n",
            type, code.value);
    return -1;
  }

  if((body_len - offset) > buf_len) {
    LOG_ERR("6P-pkt: cannot get payload [type=%u, code=%u], ",
            type, code.value);
    printf("6P-pkt: cannot get payload [type=%u, code=%u], ",
            type, code.value);
    LOG_ERR_("buf_len is too short\n");
    printf("body=%d buff=%d offset=%d\n",(int)body_len,(int)buf_len,(int)offset);
    return -1;
  }

  /*
   * Copy the content in the Payload field as it is since 6P has no idea about
   * the internal structure of the field.
   */
    printf("omid:get payload %d\n",(int)offset);
  memcpy(buf, body + offset, buf_len);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_parse(const uint8_t *buf, uint16_t len,
               sixp_pkt_t *pkt)
{
  assert(buf != NULL && pkt != NULL);
  if(buf == NULL || pkt == NULL) {
    LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of invalid argument\n");
    return -1;
  }

  memset(pkt, 0, sizeof(sixp_pkt_t));

  /* read the first 4 octets */
  if(len < 4) {
    LOG_ERR("6P-pkt: sixp_pkt_parse() fails because it's a too short packet\n");
    return -1;
  }

  /* parse the header as it's version 0 6P packet */
  pkt->version = buf[0] & 0x0f;
  pkt->type = (buf[0] & 0x30) >> 4;
  pkt->code.value = buf[1];
  pkt->sfid = buf[2];
  //printf("sfid:%02x\n",buf[2]);
  pkt->seqno = buf[3];

  buf += 4;
  len -= 4;

  LOG_INFO("6P-pkt: sixp_pkt_parse() is processing [type:%u, code:%u, len:%u]\n",
           pkt->type, pkt->code.value, len);

  /* the rest is message body called "Other Fields" */
  if(pkt->type == SIXP_PKT_TYPE_REQUEST) {
    switch(pkt->code.cmd) {
      case SIXP_PKT_CMD_ADD:
      case SIXP_PKT_CMD_DELETE:
        /* Add and Delete has the same request format */
        if(len < (sizeof(sixp_pkt_metadata_t) +
                  sizeof(sixp_pkt_cell_options_t) +
                  sizeof(sixp_pkt_num_cells_t)) ||
           (len % sizeof(uint32_t)) != 0) {
          LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;/*
      case SIXP_PKT_CMD_ADD_ADV:
        if(len < (sizeof(sixp_pkt_metadata_t) +
                  sizeof(sixp_pkt_cell_options_t) +
                  sizeof(sixp_pkt_num_cells_t)) ||
           (len % sizeof(uint32_t)) != 0) {
          LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;*/
      case SIXP_PKT_CMD_ADD_UPLINKS:
       // printf("len=%d %d\n",len, sizeof(sixp_pkt_metadata_t) + 6);
        if( len < (sizeof(sixp_pkt_metadata_t) + 6) || (len % sizeof(uint32_t)) != 0 )
        {
          printf("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_CMD_ADD_DOWNLINKS:
       // printf("len=%d %d\n",len, sizeof(sixp_pkt_metadata_t) + 6);
        if( len < (sizeof(sixp_pkt_metadata_t) + 6) || (len % sizeof(uint32_t)) != 0 )
        {
          printf("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_CMD_ASK_ADV_LINK:
       // printf("len=%d %d\n",len, sizeof(sixp_pkt_metadata_t) + 6);
        if( len < (sizeof(sixp_pkt_metadata_t) + 6) || (len % sizeof(uint32_t)) != 0 )
        {
          printf("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_CMD_ASK_CHANNEL:
         //printf("len1=%d  %d\n",len,sizeof(sixp_pkt_metadata_t)+2);
        if( len < (sizeof(sixp_pkt_metadata_t) + 2) || (len % sizeof(uint32_t)) != 0 )
        {
          LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_CMD_RELOCATE:
        if(len < (sizeof(sixp_pkt_metadata_t) +
                  sizeof(sixp_pkt_cell_options_t) +
                  sizeof(sixp_pkt_num_cells_t)) ||
           (len % sizeof(uint32_t)) != 0) {
          LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_CMD_COUNT:
        if(len != (sizeof(sixp_pkt_metadata_t) +
                   sizeof(sixp_pkt_cell_options_t))) {
          LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_CMD_LIST:
        if(len != (sizeof(sixp_pkt_metadata_t) +
                   sizeof(sixp_pkt_cell_options_t) +
                   sizeof(sixp_pkt_reserved_t) +
                   sizeof(sixp_pkt_offset_t) +
                   sizeof(sixp_pkt_max_num_cells_t))) {
          LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_CMD_SIGNAL:
        if(len < sizeof(sixp_pkt_metadata_t)) {
          LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_CMD_CLEAR:
        if(len != sizeof(sixp_pkt_metadata_t)) {
          LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      default:
        LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of unsupported cmd\n");
        return -1;
    }
  } else if(pkt->type == SIXP_PKT_TYPE_RESPONSE ||
            pkt->type == SIXP_PKT_TYPE_CONFIRMATION) {
    switch(pkt->code.rc) {
      case SIXP_PKT_RC_SUCCESS:
        /*
         * The "Other Field" contains
         * - Res to CLEAR:             Empty (length 0)
         * - Res to STATUS:            "Num. Cells" (total_num_cells)
         * - Res to ADD, DELETE, LIST: 0, 1, or multiple 6P cells
         * - Res to SIGNAL:            Payload (arbitrary length)
         */
        /* we accept any length because of SIGNAL */
        break;
      case SIXP_PKT_RC_EOL:
        if((len % sizeof(uint32_t)) != 0) {
          LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      case SIXP_PKT_RC_ERR:
      case SIXP_PKT_RC_RESET:
      case SIXP_PKT_RC_ERR_VERSION:
      case SIXP_PKT_RC_ERR_SFID:
      case SIXP_PKT_RC_ERR_SEQNUM:
      case SIXP_PKT_RC_ERR_CELLLIST:
      case SIXP_PKT_RC_ERR_BUSY:
      case SIXP_PKT_RC_ERR_LOCKED:
        if(len != 0) {
          LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of invalid length\n");
          return -1;
        }
        break;
      default:
        LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of unsupported code\n");
        return -1;
    }
  } else {
    LOG_ERR("6P-pkt: sixp_pkt_parse() fails because of unsupported type\n");
    return -1;
  }

  pkt->body = buf;
  pkt->body_len = len;

  return 0;
}
/*---------------------------------------------------------------------------*/
int
sixp_pkt_create(sixp_pkt_type_t type, sixp_pkt_code_t code,
                uint8_t sfid, uint8_t seqno,
                const uint8_t *body, uint16_t body_len, sixp_pkt_t *pkt)
{
  uint8_t *hdr;

  assert((body == NULL && body_len == 0) || (body != NULL && body_len > 0));
  if((body == NULL && body_len > 0) || (body != NULL && body_len == 0)) {
    LOG_ERR("6P-pkt: sixp_pkt_create() fails because of invalid argument\n");
    return -1;
  }

  packetbuf_clear();

  /*
   * We're going to create a packet having 6top IE header (4 octets) and body
   * (body_len octets).
   */
  if(PACKETBUF_SIZE < (packetbuf_totlen() + body_len)) {
    LOG_ERR("6P-pkt: sixp_pkt_create() fails because body is too long\n");
    return -1;
  }

  if(packetbuf_hdralloc(4) != 1) {
    LOG_ERR("6P-pkt: sixp_pkt_create fails to allocate header space\n");
    return -1;
  }
  hdr = packetbuf_hdrptr();
  /* header: write the 6top IE header, 4 octets */
  hdr[0] = (type << 4) | SIXP_PKT_VERSION;
  hdr[1] = code.value;
  hdr[2] = sfid;
  hdr[3] = seqno;

  /* data: write body */
  if(body_len > 0 && body != NULL) {
    memcpy(packetbuf_dataptr(), body, body_len);
    packetbuf_set_datalen(body_len);
  }

  /* copy information of a sending packet into pkt if necessary */
  if(pkt != NULL) {
    pkt->type = type;
    pkt->code = code;
    pkt->sfid = sfid;
    pkt->seqno = seqno;
    pkt->body = body;
    pkt->body_len = body_len;
  }

  /* packetbuf is ready to be sent */
  return 0;
}
/*---------------------------------------------------------------------------*/
/** @} */
