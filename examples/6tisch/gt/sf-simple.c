/**
 * This file contains the code for creating the TSCH schedule in the GT-TSCH protocol.
 * Developed by Omid Tavallaie.
 * 
 * The GT-TSCH protocol aims to enhance synchronous communication in low-power IoT networks
 * by implementing efficient time-slotted channel hopping mechanisms. This client
 * code is designed to run on Contiki-NG operating system, leveraging its networking
 * stack to demonstrate the protocol's capabilities in a practical scenario.
 */

#include "contiki-lib.h"
#include "node-id.h"
#include "lib/assert.h"
#include "lib/list.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/sixtop/sixtop.h"
#include "net/mac/tsch/sixtop/sixp.h"
#include "net/mac/tsch/sixtop/sixp-nbr.h"
#include "net/mac/tsch/sixtop/sixp-pkt.h"
#include "net/mac/tsch/sixtop/sixp-trans.h"

#include "sf-simple.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"



static const uint16_t slotframe_handle = 0;
static uint8_t res_storage[4 + SF_SIMPLE_MAX_LINKS * 4];
static uint8_t req_storage[4 + SF_SIMPLE_MAX_LINKS * 4];
int check_adv_link = 0;

static uint8_t two_hop_channel[8];
static linkaddr_t children_address[8];


static void read_cell(const uint8_t *buf, sf_simple_cell_t *cell);
//static void print_cell_list(const uint8_t *cell_list, uint16_t cell_list_len);
static void add_uplinks_to_schedule(const linkaddr_t *peer_addr,
                                  uint8_t link_option,
                                  const uint8_t *cell_list,
                                  uint16_t cell_list_len);
static void add_uplink_response_sent_callback(void *arg, uint16_t arg_len,
                                       const linkaddr_t *dest_addr,
                                       sixp_output_status_t status);
static void add_adv_link_response_sent_callback(void *arg, uint16_t arg_len,
                                       const linkaddr_t *dest_addr,
                                       sixp_output_status_t status);
                    
static void add_uplink_req_input(const uint8_t *body, uint16_t body_len, const linkaddr_t *peer_addr);

static void input(sixp_pkt_type_t type, sixp_pkt_code_t code,
                  const uint8_t *body, uint16_t body_len,
                  const linkaddr_t *src_addr);
static void request_input(sixp_pkt_cmd_t cmd,
                          const uint8_t *body, uint16_t body_len,
                          const linkaddr_t *peer_addr);
static void response_input(sixp_pkt_rc_t rc,
                           const uint8_t *body, uint16_t body_len,
                           const linkaddr_t *peer_addr);
                           
static uint16_t find_two_hop_frequency();




static void
read_cell(const uint8_t *buf, sf_simple_cell_t *cell)
{

  cell->timeslot_offset = buf[0] + (buf[1] << 8);
  cell->channel_offset = buf[2] + (buf[3] << 8);
}


static void
add_uplinks_to_schedule(const linkaddr_t *peer_addr, uint8_t link_option,
                      const uint8_t *cell_list, uint16_t cell_list_len)
{
 

  
  sf_simple_cell_t cell;
  struct tsch_slotframe *slotframe;
  int i;

  assert(cell_list != NULL);

  slotframe = tsch_schedule_get_slotframe_by_handle(0);

  if(slotframe == NULL) {
    return;
  }
 
  if(cell_list_len>0)
  {
      for(i = 0; i < cell_list_len; i += sizeof(cell)) 
      {
            read_cell(&cell_list[i], &cell);
            tsch_schedule_add_link(slotframe,
                                 link_option, LINK_TYPE_NORMAL, peer_addr,
                                 cell.timeslot_offset, cell.channel_offset);
            
        
        
      }
      
    
  }

  
}



static void
add_downlinks_to_schedule(const linkaddr_t *peer_addr, uint8_t link_option,
                      const uint8_t *cell_list, uint16_t cell_list_len)
{


  
  sf_simple_cell_t cell;
  struct tsch_slotframe *slotframe;
  int i;

  assert(cell_list != NULL);

  slotframe = tsch_schedule_get_slotframe_by_handle(0);

  if(slotframe == NULL) {
    return;
  }
  
  int index=find_in_send_back_by_address(peer_addr);

  
  
  if(cell_list_len>0)
  {
      for(i = 0; i < cell_list_len; i += sizeof(cell)) 
      {
            read_cell(&cell_list[i], &cell);
            tsch_schedule_add_link(slotframe,
                                 link_option, LINK_TYPE_NORMAL, peer_addr,
                                 cell.timeslot_offset, cell.channel_offset);
            if(index>=0)
            {
                        if(send_back_number_links[index]>0)
                        {  
                           
                                send_back_number_links[index] = send_back_number_links[index] -1;
                
                
                        }
            }
        
      }
    
  }


  
}


static int
delete_downlinks_from_schedule(const linkaddr_t *peer_addr,
                      const uint8_t *cell_list, uint16_t cell_list_len)
{

  
  sf_simple_cell_t cell;
  struct tsch_slotframe *slotframe;
  int i;

  assert(cell_list != NULL);

  slotframe = tsch_schedule_get_slotframe_by_handle(0);

  if(slotframe == NULL) {
    return -1;
  }

  int out=0;
  
  if(cell_list_len>0)
  {
      for(i = 0; i < cell_list_len; i += sizeof(sf_simple_cell_t)) 
      {
            read_cell(&cell_list[i], &cell);
            int out=tsch_schedule_delete_link(slotframe, LINK_OPTION_RX, LINK_TYPE_NORMAL, peer_addr, cell.timeslot_offset, cell.channel_offset);
            if( out != 1 )
            {
                                     printf("cannot delete the link\n");
            }
            else
            {
                out++;
            }
                               
        
      }
      
  }
  
   return out; 
   
   
  
}


static void
delete_uplinks_from_schedule(const linkaddr_t *peer_addr, const uint8_t *cell_list, uint16_t cell_list_len, uint8_t *cell_list_out, uint16_t* cell_list_len_out)
{
  
	  sf_simple_cell_t cell;
	  
	  sf_simple_cell_t cell_out;
	  struct tsch_slotframe *slotframe;
	  int i;

	  assert(cell_list != NULL);

	  slotframe = tsch_schedule_get_slotframe_by_handle(0);

	  if(slotframe == NULL) 
	  {
		return;
	  }
	  
	  
	  
	  int counter=0;
	  if(cell_list_len>0)
	  {
		  for(i = 0; i < cell_list_len; i += sizeof(cell)) 
		  {
				read_cell(&cell_list[i], &cell);
				if(tsch_schedule_delete_link(slotframe,
									 LINK_OPTION_TX, LINK_TYPE_NORMAL, peer_addr,
									 cell.timeslot_offset, cell.channel_offset)!=1)
									 {
										 printf("cannot delete the link\n");
									 }
									 else
									 {
										 if(dtsf_check_RX_timeslot(slotframe, cell.timeslot_offset - 1, children_channel)==1)
										 {
											 cell_out.channel_offset = children_channel;
											 cell_out.timeslot_offset= cell.timeslot_offset - 1;
											 memcpy(cell_list_out + counter, &cell_out,sizeof(sf_simple_cell_t));
											  counter=counter + sizeof(sf_simple_cell_t);
										 }
										
									 }
			
		  }
		  
		  
	  }

	  *cell_list_len_out=counter;
}


static void
add_adv_link_to_schedule(const linkaddr_t *peer_addr, uint8_t link_option,
                      const uint8_t *cell_list, uint16_t cell_list_len)
{

	  check_adv_link = 1;
	  sf_simple_cell_t cell;
	  struct tsch_slotframe *slotframe;
	  int i;

	  assert(cell_list != NULL);

	  slotframe = tsch_schedule_get_slotframe_by_handle(0);

	  if(slotframe == NULL) {
		return;
	  }
	 
	  for(i = 0; i < cell_list_len; i += sizeof(cell)) 
	  {
			read_cell(&cell_list[i], &cell);
			printf("adding adv slot\n");
			tsch_schedule_add_link(slotframe,
								 link_option, LINK_TYPE_NORMAL, peer_addr,
								 cell.timeslot_offset, cell.channel_offset);
			adv_timeslots++;
			if(tsch_is_coordinator)
			{
			
				free_uplink_timeslots=free_uplink_timeslots -1;
			   
			}

		
	  }
  
}



static void
add_uplink_response_sent_callback(void *arg, uint16_t arg_len,
                           const linkaddr_t *dest_addr,
                           sixp_output_status_t status)
{
   
  uint8_t *body = (uint8_t *)arg;
  uint16_t body_len = arg_len;
  const uint8_t *cell_list;
  uint16_t cell_list_len;
  sixp_nbr_t *nbr;

  assert(body != NULL && dest_addr != NULL);

  if(status == SIXP_OUTPUT_STATUS_SUCCESS &&
     sixp_pkt_get_cell_list_for_uplink(SIXP_PKT_TYPE_RESPONSE,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                            &cell_list, &cell_list_len,
                            body, body_len) == 0 &&
     (nbr = sixp_nbr_find(dest_addr)) != NULL) {
    
    add_downlinks_to_schedule(dest_addr, LINK_OPTION_RX,
                          cell_list, cell_list_len);
  }
}


static void
add_downlink_request_sent_callback(void *arg, uint16_t arg_len,
                           const linkaddr_t *dest_addr,
                           sixp_output_status_t status)
{
   
	  uint8_t *body = (uint8_t *)arg;
	  uint16_t body_len = arg_len;
	  const uint8_t *cell_list;
	  uint16_t cell_list_len;
	  sixp_nbr_t *nbr;

	  assert(body != NULL && dest_addr != NULL);

	  if(status == SIXP_OUTPUT_STATUS_SUCCESS &&
		 sixp_pkt_get_cell_list_for_downlink(SIXP_PKT_TYPE_REQUEST,
								(sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD_DOWNLINKS,
								&cell_list, &cell_list_len,
								body, body_len) == 0 &&
		 (nbr = sixp_nbr_find(dest_addr)) != NULL) {
					add_downlinks_to_schedule(dest_addr, LINK_OPTION_RX,cell_list, cell_list_len);
	  }
}

static void
delete_downlink_request_sent_callback(void *arg, uint16_t arg_len,
                           const linkaddr_t *dest_addr,
                           sixp_output_status_t status)
{
   
	  uint8_t *body = (uint8_t *)arg;
	  uint16_t body_len = arg_len;
	  const uint8_t *cell_list;
	  uint16_t cell_list_len;
	  sixp_nbr_t *nbr;

	  assert(body != NULL && dest_addr != NULL);

	  if(status == SIXP_OUTPUT_STATUS_SUCCESS &&
		 sixp_pkt_get_cell_list_for_delete_downlink(SIXP_PKT_TYPE_REQUEST,
								(sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE_DOWNLINK,
								&cell_list, &cell_list_len,
								body, body_len) == 0 &&
		 (nbr = sixp_nbr_find(dest_addr)) != NULL) 
		{

		int out= delete_downlinks_from_schedule(dest_addr,
							  cell_list, cell_list_len);
								
			int index=find_in_send_back_by_address(dest_addr);
		   if(index<0)
			{
				index=find_free_place_in_send_back();
												 
				if(index<0)
				{
					printf("Error: cannot find empty space in send back array\n");
				}
				send_back_number_links[index]=0;
				
				send_back_address[index]=*dest_addr;
			}
			  
			if(out>0)
			{  
			send_back_number_links[index] = send_back_number_links[index] +out;
			required_slots = required_slots + out;
			
			}

	  }
}

static void
delete_uplink_request_sent_callback(void *arg, uint16_t arg_len,
                           const linkaddr_t *dest_addr,
                           sixp_output_status_t status)
{
   
  uint8_t *body = (uint8_t *)arg;
  uint16_t body_len = arg_len;
  const uint8_t *cell_list;
  uint16_t cell_list_len;
  sixp_nbr_t *nbr;
  uint8_t cell_list_out[96];
  uint16_t cell_list_len_out=0;
  assert(body != NULL && dest_addr != NULL);

  if(status == SIXP_OUTPUT_STATUS_SUCCESS &&
     sixp_pkt_get_cell_list_for_delete_uplink(SIXP_PKT_TYPE_REQUEST,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE_UPLINK,
                            &cell_list, &cell_list_len,
                            body, body_len) == 0 &&
     (nbr = sixp_nbr_find(dest_addr)) != NULL) {
		 delete_uplinks_from_schedule(dest_addr, cell_list, cell_list_len, &cell_list_out[0],&cell_list_len_out);
		 if(cell_list_len_out > 0)
		 {
			int i=0;
			for(i=0;i<cell_list_len_out; i = i+sizeof(sf_simple_cell_t))
			{
				
				required_slots--;
			   
				
			}
		}
	}
  
  
}

static void
add_adv_link_response_sent_callback(void *arg, uint16_t arg_len,
                           const linkaddr_t *dest_addr,
                           sixp_output_status_t status)
{
  
	  uint8_t *body = (uint8_t *)arg;
	  uint16_t body_len = arg_len;
	  const uint8_t *cell_list;
	  uint16_t cell_list_len;
	  sixp_nbr_t *nbr;
	  int i;
	  sf_simple_cell_t cell;
	  assert(body != NULL && dest_addr != NULL);

	  if(status == SIXP_OUTPUT_STATUS_SUCCESS &&
		 sixp_pkt_get_cell_list_for_adv_link(SIXP_PKT_TYPE_RESPONSE,
								(sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
								&cell_list, &cell_list_len,
								body, body_len) == 0 &&
		 (nbr = sixp_nbr_find(dest_addr)) != NULL) 
	   {
		
			add_adv_link_to_schedule(dest_addr, (LINK_OPTION_TX | LINK_OPTION_RX), cell_list, cell_list_len);
			for(i = 0; i < cell_list_len; i += sizeof(cell)) 
			{
				read_cell(&cell_list[i], &cell);
				int out= delete_from_cell_list_reserved(cell.timeslot_offset,cell.channel_offset);
				if(out!=1)
				{
					printf("error: cannot delete slot\n");
				}
			
			}
	  }
}

void set_children_channel(const linkaddr_t *dest_addr,int channel)
{
    int i;
    for(i=0;i<8;i++)
    {
        if(two_hop_channel[i]==0)
        {
            two_hop_channel[i]=channel;
            children_address[i]=*dest_addr;
            return;
        }
            
    }
    
    printf("error: no empty space in two_hop_channel\n");
}

static void
channel_response_sent_callback(void *arg, uint16_t arg_len,
                           const linkaddr_t *dest_addr,
                           sixp_output_status_t status)
{
 
	  uint8_t *body = (uint8_t *)arg;
	  uint16_t body_len = arg_len;
	  uint16_t channel;
	  sixp_nbr_t *nbr;

	  assert(body != NULL && dest_addr != NULL);

	  if(status == SIXP_OUTPUT_STATUS_SUCCESS &&
		 sixp_pkt_get_channel(SIXP_PKT_TYPE_RESPONSE,
								(sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
								body, body_len) != -1 &&
		 (nbr = sixp_nbr_find(dest_addr)) != NULL) 
		 {
			// printf("callback for channel channel=%d\n",channel);
			 channel=sixp_pkt_get_channel(SIXP_PKT_TYPE_RESPONSE, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS, body, body_len);
			 set_children_channel(dest_addr,channel);
			 struct tsch_neighbor *n = NULL;
			 n = tsch_queue_get_nbr(dest_addr);
			 n->frequency_offset=channel;
			 n->is_child=1;
		 }
}



static void
add_uplink_req_input(const uint8_t *body, uint16_t body_len, const linkaddr_t *peer_addr)
{

 // printf("receive uplink req\n");
  sf_simple_cell_t cell;
  struct tsch_slotframe *slotframe;


                      
  //assert(body != NULL && peer_addr != NULL);
  if(body==NULL && peer_addr==NULL)
  {
      return;
  }
  uint32_t number_of_cells = sixp_pkt_get_request_add_uplink(SIXP_PKT_TYPE_REQUEST,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD_UPLINKS,
                            body, body_len);
  //printf("number of cells 1=%d\n",(int)number_of_cells);
  if(number_of_cells<=0)
  {
     printf("error13: Parse error on add uplinks request\n");
    return;
  }
                            

 
   uint16_t channel_offset= children_channel;
   slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);
   if(slotframe == NULL) 
   {
    printf("slotframe is null\n");
    return;
   }


   sf_simple_cell_t cell_list[SF_SIMPLE_MAX_LINKS];
   uint32_t index=0;

  // printf("number of cells=%d\n",(int)number_of_cells);
  if(need_send_back==0)
  {
    index = dtsf_find_free_uplink_slot(slotframe, channel_offset, number_of_cells, cell_list, peer_addr ); 
  }
   // printf("index=%d\n",(int)index);
   
   if(allocate_slot_for_packet_generation==0)
   {
       index=0;
                       printf("error: cannot dedicate timeslot as node needs them for the packet generation\n");

   }
  
    
    if (index < 1)
    {
        // printf("Could not find any free uplinks\n"); 
         
         
    }
    
    if(!tsch_is_coordinator)
    {
        if(index < number_of_cells)
        {

            
            
                 need_send_back=1;
                 int j = find_in_send_back_by_address(peer_addr);
                 if(j>=0)
                 {
                     if(send_back_number_links[j]==0)
                     {
                                     required_slots = required_slots + (number_of_cells - index);
                       send_back_number_links[j]= send_back_number_links[j] + number_of_cells - index;
                     }
                 }
                 else
                 {
                        j=find_free_place_in_send_back();
                        if(j>=0)
                        {
                                        required_slots = required_slots + (number_of_cells - index);
                            send_back_number_links[j]= number_of_cells - index;
                            send_back_address[j]= *peer_addr;
                        }
                        else
                        {
                            
                            printf("error in finding empty space in Send_Back_Array\n");
                            printf("j=%d",j);
                        }
                     
                 }
                 
   
           
        }
        
    }
       
     
      memset(res_storage, 0, sizeof(res_storage));
      uint16_t res_len = 4;
      sixp_pkt_set_response_uplinks(SIXP_PKT_TYPE_RESPONSE, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS, index,
                       res_storage, sizeof(res_storage));
      

      int i=0;
      
      for(i = 0;  i < index; i += 1) 
      {
         read_cell((uint8_t*)&cell_list[i], &cell);
         
         sixp_pkt_set_cell_list_for_uplink(SIXP_PKT_TYPE_RESPONSE,
                                     (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                                     (uint8_t *)&cell, sizeof(cell),
                                     i,
                                     res_storage, sizeof(res_storage));
                                     
       
          res_len += sizeof(cell);

      }
      
  
  
      
     
          printf("sending uplink response\n");
      sixp_output(SIXP_PKT_TYPE_RESPONSE,
                  (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                  SF_SIMPLE_SFID,
                  res_storage, res_len, peer_addr,
                  add_uplink_response_sent_callback, res_storage, res_len);

}




static void
add_downlink_req_input(const uint8_t *body, uint16_t body_len, const linkaddr_t *peer_addr)
{
         const uint8_t *cell_list;
         uint16_t cell_list_len;
        if(sixp_pkt_get_cell_list_for_downlink(SIXP_PKT_TYPE_RESPONSE,
                                  (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                                  &cell_list, &cell_list_len,
                                  body, body_len) != 0) {
                                      
          
          printf("error1: Parse error on add-dwonlink request\n");
          return;
        }

        if(cell_list_len>0)
        {
              check_ask_uplink=1;
            add_uplinks_to_schedule(peer_addr, LINK_OPTION_TX, cell_list, cell_list_len);
        }
        else
        {
            check_ask_uplink=0;
            printf("error in  the request of add_down_link\n");
        }
}

static void
delete_downlink_req_input(const uint8_t *body, uint16_t body_len, const linkaddr_t *peer_addr)
{
    
    sf_simple_cell_t cell_list_for_trans[24];
     sf_simple_cell_t cell_list_for_process[24];
    linkaddr_t *peer_addr_for_trans;
         const uint8_t *cell_list;
         uint16_t cell_list_len=0;
          int i=0;
          int j=0;
          uint16_t counter=0;
           uint8_t cell_list_out[96];
          
          
        if(sixp_pkt_get_cell_list_for_delete_downlink(SIXP_PKT_TYPE_REQUEST,
                                  (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE_DOWNLINK,
                                  &cell_list, &cell_list_len,
                                  body, body_len) != 0) {
                                      
          
          printf("error1: Parse error on add-dwonlink request\n");
          return;
        }

        if(cell_list_len>0)
        {
         
            uint16_t cell_list_len_out=0;
            check_ask_uplink=0;
            delete_uplinks_from_schedule(peer_addr, cell_list, cell_list_len, &cell_list_out[0], &cell_list_len_out);
            if(cell_list_len_out > 0)
            {
                j=0;
                for(i=0;i<cell_list_len_out; i = i+sizeof(sf_simple_cell_t))
                {
                    
                    required_slots++;
                    memcpy(cell_list_out+i, &cell_list_for_process[j],sizeof(sf_simple_cell_t));
                    j++;
                    
                }
            }
            
        
                
            cell_list_len_out=j;
            for(i=0;i<cell_list_len_out;i++)
            {
                      if(cell_list_for_process[i].timeslot_offset!=-1)
                      {
                          struct tsch_link* l= tsch_schedule_get_link_by_timeslot(0,cell_list_for_process[i].timeslot_offset, cell_list_for_process[i].channel_offset); 
                          if(l != NULL)
                          {
                                  peer_addr_for_trans=&l->addr;
                                  counter=0;
                                  cell_list_for_trans[counter].channel_offset=cell_list_for_process[i].channel_offset;
                                  cell_list_for_trans[counter].timeslot_offset=cell_list_for_process[i].timeslot_offset;
                                  cell_list_for_process[i].timeslot_offset=-1;
                                  counter++;
                                   for (j=i+1;j<cell_list_len_out;j++)
                                   {
                                        if(cell_list_for_process[j].timeslot_offset!= -1)
                                        {
                                            l= tsch_schedule_get_link_by_timeslot(0,cell_list_for_process[j].timeslot_offset, cell_list_for_process[j].channel_offset);
                                            if(l!=NULL)
                                            {
                                                if(linkaddr_cmp(&l->addr, peer_addr_for_trans))
                                                {
                                                   cell_list_for_trans[counter].channel_offset=cell_list_for_process[i].channel_offset;
                                                   cell_list_for_trans[counter].timeslot_offset=cell_list_for_process[i].timeslot_offset;
                                                   cell_list_for_process[i].timeslot_offset=-1;
                                                   counter ++;
                                                }
                                                
                                            }
                                            else
                                            {
                                                printf("error: downlink cannot be found\n");
                                            }
                                            
                                        }
                                        
                                    
                                   }
                                   
                                   if(counter>0)
                                   {
                                       dtsf_send_delete_downlink(peer_addr_for_trans, &cell_list_for_trans[0], counter);
                                   }
                          
                          }
                          else
                          {
                              printf("error for finding a downlink\n");
                          }
                    
                    }
                  
                      
                      
            }
                
        }

}



static void
delete_uplink_req_input(const uint8_t *body, uint16_t body_len, const linkaddr_t *peer_addr)
{
    
    
         const uint8_t *cell_list;
         uint16_t cell_list_len=0;
         
          int j=0;
         
         
          
          
        if(sixp_pkt_get_cell_list_for_delete_downlink(SIXP_PKT_TYPE_REQUEST,
                                  (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE_UPLINK,
                                  &cell_list, &cell_list_len,
                                  body, body_len) != 0) {
                                      
          
          printf("error1: Parse error on add-dwonlink request\n");
          return;
        }

        if(cell_list_len>0)
        {
         
         
           
            delete_downlinks_from_schedule(peer_addr, cell_list, cell_list_len);
            
        }
        
       j= find_in_send_back_by_address(peer_addr);
       
       if(j>0)
       {
           required_slots = required_slots - send_back_number_links[j];
           send_back_number_links[j]=0;
       }

}


void  dtsf_send_add_downlink()
{
    
        sf_simple_cell_t cell;
        
        int i=find_first_place_in_send_back();
        if(i<0)
        {
           // printf("Error: Cannot find any send back cases\n");
            return;
        }
        
        printf("start sendback procedure\n");
        
        int number_of_cells= send_back_number_links[i];
        
        
       // printf("number=%d",number_of_cells);
    
    
         uint8_t req_len;
         
          memset(req_storage, 0, sizeof(req_storage));
         
         
 
   uint16_t channel_offset= children_channel;
   struct tsch_slotframe *slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);
   if(slotframe == NULL) 
   {
    printf("slotframe is null\n");
    return;
   }


   sf_simple_cell_t cell_list[SF_SIMPLE_MAX_LINKS];
   uint32_t index=0;

  // printf("number of cells=%d\n",(int)number_of_cells);
    index = dtsf_find_free_uplink_slot(slotframe, channel_offset, number_of_cells, cell_list, &send_back_address[i]); 
    //printf("index=%d\n",(int)index);
   
    if (index < 1)
    {
        if(tsch_is_coordinator)
        {
            printf("Master node does not have any free uplink\n"); 
        }
         printf("Could not find any free uplink for send back\n"); 
         
         
         return;
    }
    
      memset(req_storage, 0, sizeof(req_storage));

      sixp_pkt_set_request_add_downlink(SIXP_PKT_TYPE_REQUEST, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD_DOWNLINKS, index,
                       req_storage, sizeof(req_storage));
      
      req_len = 8;
      int j=0;
      for(j = 0;  j < index; j ++) 
      {
         read_cell((uint8_t*)&cell_list[j], &cell);
         
        // printf("rrr=%d\n",cell_list[i].timeslot_offset);
         sixp_pkt_set_cell_list_for_downlink(SIXP_PKT_TYPE_REQUEST,
                                     (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD_DOWNLINKS,
                                     (uint8_t *)&cell, sizeof(cell),
                                     j,
                                     req_storage, sizeof(req_storage));
                                     
       
          req_len += sizeof(cell);

      }
      printf("Send an add downlink to child %d\n", send_back_address[i].u8[7]);
     
      sixp_output(SIXP_PKT_TYPE_REQUEST, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD_DOWNLINKS,
                  SF_SIMPLE_SFID,
                  req_storage, req_len, &send_back_address[i],
                  add_downlink_request_sent_callback, req_storage, req_len);
    
}


void  dtsf_send_delete_downlink(const linkaddr_t* peer_addr, sf_simple_cell_t* cell_list, uint16_t cell_list_len)
{
    
        sf_simple_cell_t cell;
      
        
        printf("start delete downlink procedure\n");
    
         uint8_t req_len;
         

    
      memset(req_storage, 0, sizeof(req_storage));

      sixp_pkt_set_request_delete_downlinks(SIXP_PKT_TYPE_REQUEST, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE_DOWNLINK, cell_list_len,
                       req_storage, sizeof(req_storage));
      
      req_len = 8;
      int j=0;
      for(j = 0;  j < cell_list_len; j += 1) 
      {
         read_cell((uint8_t*)&cell_list[j], &cell);
         
        sixp_pkt_set_cell_list_for_delete_downlink(SIXP_PKT_TYPE_REQUEST,
                                     (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE_DOWNLINK,
                                     (uint8_t *)&cell, sizeof(cell),
                                     j,
                                     req_storage, sizeof(req_storage));
                                     
       
          req_len += sizeof(cell);

      }
      printf("Send a delete downlink request to node %d\n", peer_addr->u8[7]);
     
      sixp_output(SIXP_PKT_TYPE_REQUEST, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE_DOWNLINK,
                  SF_SIMPLE_SFID,
                  req_storage, req_len, peer_addr,
                  delete_downlink_request_sent_callback, req_storage, req_len);
    
}


int dtsf_send_add_uplink(const linkaddr_t* peer_addr, uint32_t number_of_links)
{
  uint8_t req_len;
  assert(peer_addr != NULL);   
  memset(req_storage, 0, sizeof(req_storage));
 // printf("end2460=%d",sizeof(req_storage));
  if(sixp_pkt_set_request_add_uplink(SIXP_PKT_TYPE_REQUEST,
                      (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD_UPLINKS,number_of_links,
                      req_storage, sizeof(req_storage)) != 0)
  {
    printf("sf-simple: Build error on uplink request\n");
    return -1;
  }
  
  //total length must be a multiplication of 4, so number_of_links must be uint32_t

  req_len = 8;
  /*int i=0;
  for(i=0;i<8;i++)
  {
      printf("%02x ",req_storage[i]);
  }
  printf("end2463\n");
   */
  sixp_output(SIXP_PKT_TYPE_REQUEST, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ADD_UPLINKS,
              SF_SIMPLE_SFID,
              req_storage, req_len, peer_addr,
              NULL, NULL, 0);

  //printf("sf-simple: Send a 6P ASK UPLINK to node %d \n", peer_addr->u8[7]);
  return 0;
}


void  dtsf_send_delete_uplink(const linkaddr_t* peer_addr, sf_simple_cell_t* cell_list, uint16_t cell_list_len)
{
    
        sf_simple_cell_t cell;
      
        
        printf("start delete uplink procedure\n");
    
         uint8_t req_len;
         

    
      memset(req_storage, 0, sizeof(req_storage));

      sixp_pkt_set_request_delete_uplinks(SIXP_PKT_TYPE_REQUEST, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE_UPLINK, cell_list_len,
                       req_storage, sizeof(req_storage));
      
      req_len = 8;
      int j=0;
      for(j = 0;  j < cell_list_len; j += 1) 
      {
         read_cell((uint8_t*)&cell_list[j], &cell);
         
        //printf("rrr=%d\n",cell_list[i].timeslot_offset);
        sixp_pkt_set_cell_list_for_delete_uplink(SIXP_PKT_TYPE_REQUEST,
                                     (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE_UPLINK,
                                     (uint8_t *)&cell, sizeof(cell),
                                     j,
                                     req_storage, sizeof(req_storage));
                                     
       
          req_len += sizeof(cell);

      }
      printf("Send a delete uplink request to node %d\n", peer_addr->u8[7]);
     
      sixp_output(SIXP_PKT_TYPE_REQUEST, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_DELETE_UPLINK,
                  SF_SIMPLE_SFID,
                  req_storage, req_len, peer_addr,
                  delete_uplink_request_sent_callback, req_storage, req_len);
    
}


static void
add_adv_link_req_input(const uint8_t *body, uint16_t body_len, const linkaddr_t *peer_addr)
{

  //printf("receive adv link req1\n");
  sf_simple_cell_t cell;
  struct tsch_slotframe *slotframe;
  int feasible_link;
 

  uint16_t res_len;
                      
  assert(body != NULL && peer_addr != NULL);
  uint32_t number_of_cells = sixp_pkt_get_requested_adv_link(SIXP_PKT_TYPE_REQUEST,
                            (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ASK_ADV_LINK,
                            body, body_len);
  //printf("number of cells 1=%d\n",(int)number_of_cells);
  if(number_of_cells<=0)
  {
     printf("error13: Parse error on add adv link request\n");
    return;
  }
                            

 
  // uint16_t channel_offset= children_channel;
  uint16_t channel_offset= children_channel;
   slotframe = tsch_schedule_get_slotframe_by_handle(slotframe_handle);
   if(slotframe == NULL) 
   {
    printf("slotframe is null\n");
    return;
   }
   



   sf_simple_cell_t cell_list[SF_SIMPLE_MAX_LINKS];
   uint32_t index=0;

   //printf("number of cells=%d\n",(int)number_of_cells);
    index = dtsf_find_free_adv_link_slot(slotframe, channel_offset, number_of_cells, cell_list, peer_addr );
    if (index < 1)
    {
         printf("Error: cannot find any free adv link\n"); 
        return;
    }
       
    //printf("you can index=%d\n ",(int)index);
      memset(res_storage, 0, sizeof(res_storage));
      res_len = 4;
      sixp_pkt_set_response_adv_link(SIXP_PKT_TYPE_RESPONSE, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS, index,
                       res_storage, sizeof(res_storage));
      

      int i;
      for(i = 0, feasible_link = 0;  feasible_link < index; i += 1) 
      {
         read_cell((uint8_t*)&cell_list[i], &cell);
         
         
         sixp_pkt_set_cell_list_for_adv_link(SIXP_PKT_TYPE_RESPONSE,
                                     (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                                     (uint8_t *)&cell, sizeof(cell),
                                     feasible_link,
                                     res_storage, sizeof(res_storage));
          res_len += sizeof(cell);
          feasible_link++;

      }
      
  
  
      
     // PRINTF("sf-simple: Send a 6P Response to node %d\n", peer_addr->u8[7]);
      sixp_output(SIXP_PKT_TYPE_RESPONSE,
                  (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                  SF_SIMPLE_SFID,
                  res_storage, res_len, peer_addr,
                  add_adv_link_response_sent_callback, res_storage, res_len);
    

  
}


int look_in_children_channel(const linkaddr_t *peer_addr)
{
    
    int i;
    for(i=0;i<8;i++)
    {
        
        if(linkaddr_cmp(&children_address[i],peer_addr) && two_hop_channel[i]!=0)
        {
            return two_hop_channel[i];
        }
    }
    return 0;
}


static void
ask_channel_req_input(const uint8_t *body, uint16_t body_len, const linkaddr_t *peer_addr)
{

                  
  assert(body != NULL && peer_addr != NULL);
  
  
  
  if(peer_addr->u8[7]==0)
  {
      printf("catch a bug\n");
      return;
  }
   uint16_t res_len;
   uint16_t channel;

   int ch = look_in_children_channel(peer_addr);
   if(ch!=0)
   {
        channel = ch;
       // printf("no\n");
       
   }
   else
   {
     channel= find_two_hop_frequency();
   }
  /////////////////////////////////////////////////////////
  sixp_pkt_set_channel(SIXP_PKT_TYPE_RESPONSE,
                      (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,channel,
                      res_storage, sizeof(res_storage));
   res_len=2;
      PRINTF("sf-simple: Send a 6P ASK__CHANNEL Response to node %d\n", peer_addr->u8[7]);
      if(node_id==29)
      {
            if(peer_addr->u8[7]==0)
              {
                  printf("catch a bug\n");
                  return;
              }
              printf("%02x\n",peer_addr->u8[7]);
      }
      sixp_output(SIXP_PKT_TYPE_RESPONSE,
                  (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                  SF_SIMPLE_SFID,
                  res_storage, res_len, peer_addr,
                  channel_response_sent_callback, res_storage, res_len);
  
}






static void
input(sixp_pkt_type_t type, sixp_pkt_code_t code,
      const uint8_t *body, uint16_t body_len, const linkaddr_t *src_addr)
{
  
 
  assert(body != NULL && body != NULL);
  switch(type) {
    case SIXP_PKT_TYPE_REQUEST:
      request_input(code.cmd, body, body_len, src_addr);
      break;
    case SIXP_PKT_TYPE_RESPONSE:
      response_input(code.cmd, body, body_len, src_addr);
      break;
    default:
      /* unsupported */
      break;
  }
  
 
  
}

static void
request_input(sixp_pkt_cmd_t cmd,
              const uint8_t *body, uint16_t body_len,
              const linkaddr_t *peer_addr)
{
  assert(body != NULL && peer_addr != NULL);
  linkaddr_t peer_addr1=*peer_addr;
  switch(cmd) {
	case SIXP_PKT_CMD_ASK_CHANNEL:
      printf("ask_channel_req_input from %d\n", peer_addr1.u8[7]);
      ask_channel_req_input(body, body_len, peer_addr);
      break;
	case SIXP_PKT_CMD_ADD_UPLINKS:
      printf("add_uplink_req_input from %d\n", peer_addr1.u8[7]);
      add_uplink_req_input(body, body_len, peer_addr);
      break;
    case SIXP_PKT_CMD_ADD_DOWNLINKS:
      printf("add_downlink_req_input from %d\n", peer_addr1.u8[7]);
      add_downlink_req_input(body, body_len, peer_addr);
      break;
    case SIXP_PKT_CMD_DELETE_DOWNLINK:
      printf("delete_downlink_req_input from %d\n", peer_addr1.u8[7]);
      delete_downlink_req_input(body, body_len, peer_addr);
      break;
    case SIXP_PKT_CMD_DELETE_UPLINK:
      printf("delete_uplink_req_input from %d\n", peer_addr1.u8[7]);
      delete_uplink_req_input(body, body_len, peer_addr);
      break;
    case SIXP_PKT_CMD_ASK_ADV_LINK:
      printf("ask_adv_link_req_input from %d\n", peer_addr1.u8[7]);
      add_adv_link_req_input(body, body_len, peer_addr);
      break;
    
    default:
      /* unsupported request */
      break;
  }
}
static void
response_input(sixp_pkt_rc_t rc,
               const uint8_t *body, uint16_t body_len,
               const linkaddr_t *peer_addr)
{
 // printf("receive response\n");
  const uint8_t *cell_list;
  uint16_t cell_list_len;
  sixp_nbr_t *nbr;
  sixp_trans_t *trans;
  
      

  assert(body != NULL && peer_addr != NULL);

  if((nbr = sixp_nbr_find(peer_addr)) == NULL ||
     (trans = sixp_trans_find(peer_addr)) == NULL) {
    return;
  }

  if(rc == SIXP_PKT_RC_SUCCESS) {
    switch(sixp_trans_get_cmd(trans)) {
        
        
        case SIXP_PKT_CMD_ADD_UPLINKS:
        printf("receive ADD_UPLINKS response\n");
      
        if(sixp_pkt_get_cell_list_for_uplink(SIXP_PKT_TYPE_RESPONSE,
                                  (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                                  &cell_list, &cell_list_len,
                                  body, body_len) != 0) {
                                      
          
          printf("error1: Parse error on add response\n");
          return;
        }

        if(cell_list_len>0)
        {
         
              check_ask_uplink=1;
            add_uplinks_to_schedule(peer_addr, LINK_OPTION_TX, cell_list, cell_list_len);
        }
        else
        {
          
            check_ask_uplink=0;
        }
        break;
     case SIXP_PKT_CMD_ASK_ADV_LINK:
        printf("receive ASK_ADV_LINK response\n");
     
        if(sixp_pkt_get_cell_list_for_adv_link(SIXP_PKT_TYPE_RESPONSE,
                                  (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                                  &cell_list, &cell_list_len,
                                  body, body_len) != 0) {
          
          printf("error1: Parse error on add response\n");
          return;
        }

        add_adv_link_to_schedule(peer_addr, (LINK_OPTION_TX | LINK_OPTION_RX), cell_list, cell_list_len);
        break;
  
     case SIXP_PKT_CMD_ASK_CHANNEL:
      printf("receive ASK_CHANNEL response\n");;
        if(sixp_pkt_get_channel(SIXP_PKT_TYPE_RESPONSE,
                                  (sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS,
                                  body, body_len) == -1) {
          printf("error: Parse error on ask channel response\n");
          return;
        }
       
       children_channel = (uint8_t)sixp_pkt_get_channel(SIXP_PKT_TYPE_RESPONSE,(sixp_pkt_code_t)(uint8_t)SIXP_PKT_RC_SUCCESS, body, body_len);
       
      // printf("children tttt=%d\n", children_channel);
      //  printf("sf-simple: Received a 6P ASK CHANNEL Response   channel=%d\n",children_channel);
        break;
      case SIXP_PKT_CMD_COUNT:
      case SIXP_PKT_CMD_LIST:
      case SIXP_PKT_CMD_CLEAR:
      default:
        PRINTF("error: unsupported response\n");
    }
  }
}

int check_children_channel(int in)
{
    int i;
    for(i=0;i<8;i++)
    {
        if(two_hop_channel[i]==in)
        {
            return 1;
        }
        
    }
    
    return 0;
}

static uint16_t
find_two_hop_frequency()
{
    
   
     if(children_channel==0 || parent_channel==0)
     {
          printf("error:children_chann=%d , parent_channel=%d",children_channel,parent_channel);
         return -1;
     }
   
	int rand= random_rand()%tsch_hopping_sequence_length.val;
    int i;
    for(i=rand;i<tsch_hopping_sequence_length.val;i++)
    {
       
        if(i!=parent_channel  && i!=default_channel  && i!=children_channel)
        {
            //printf("just_before_check  %d\n",tsch_hopping_sequence_length.val);

            if(check_children_channel(i)==0)
            {
             
            //  printf("children channe=%d\n",i);
              
              return i;  
                
            }
        
            
        }
        
    }
    
    
    for(i=rand;i>-1;i--)
    {
       
        if(i!=parent_channel  && i!=default_channel  && i!=children_channel)
        {
            //printf("just_before_check  %d\n",tsch_hopping_sequence_length.val);

            if(check_children_channel(i)==0)
            {
             
           //   printf("children channe=%d\n",i);
              
              return i;  
                
            }
        
            
        }
        
    }
    
    
    printf("Error in selecting two-hop channel\n");
    return -1;
      
}





int 
dtsf_ask_channel(const linkaddr_t* peer_addr)
{

  uint8_t req_len;
  assert(peer_addr != NULL);   
  memset(req_storage, 0, sizeof(req_storage));
  
  req_len = 4;
  sixp_output(SIXP_PKT_TYPE_REQUEST, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ASK_CHANNEL,
              SF_SIMPLE_SFID,
              req_storage, req_len, peer_addr,
              NULL, NULL, 0);

  printf("sf-simple: Send a 6P ASK CHANNEL to node %d \n", peer_addr->u8[7]);
  return 0;
}

int dtsf_ask_adv_link(const linkaddr_t* peer_addr, uint32_t number_of_links)
{
  uint8_t req_len;
  assert(peer_addr != NULL);   
  memset(req_storage, 0, sizeof(req_storage));
  if(sixp_pkt_set_requested_adv_link(SIXP_PKT_TYPE_REQUEST,
                      (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ASK_ADV_LINK,number_of_links,
                      req_storage, sizeof(req_storage)) != 0)
  {
    printf("sf-simple: Build error on uplink request\n");
    return -1;
  }
  
  //total length must be a multiplication of 4, so number_of_links must be uint32_t

  req_len = 8;

  sixp_output(SIXP_PKT_TYPE_REQUEST, (sixp_pkt_code_t)(uint8_t)SIXP_PKT_CMD_ASK_ADV_LINK,
              SF_SIMPLE_SFID,
              req_storage, req_len, peer_addr,
              NULL, NULL, 0);

  printf("sf-simple: Send a 6P ASK ADV LINK to node %d \n", peer_addr->u8[7]);
  return 0;
}




static void
timeout(sixp_pkt_cmd_t cmd, const linkaddr_t *peer_addr)
{

}

static void
init()
{
    int i;
    for(i=0;i<8;i++)
    {
        two_hop_channel[i]=0;
    }
   // printf("init\n");
   if(tsch_is_coordinator)
   {
       children_channel=node_id%8;
       //printf("set parent channel\n");
       parent_channel=children_channel;
       
       shared_timeslot_children=TSCH_SCHEDULE_CONF_DEFAULT_LENGTH-1;
       shared_timeslot_parent=TSCH_SCHEDULE_CONF_DEFAULT_LENGTH-1;
      
       allocate_slot_for_packet_generation=1;
       
       free_uplink_timeslots=TSCH_SCHEDULE_CONF_DEFAULT_LENGTH;
       
       check_shared_timeslot=1;
       
   }
  
}


const sixtop_sf_t sf_simple_driver = {
  SF_SIMPLE_SFID,
  CLOCK_SECOND*2,
  init,
  input,
  timeout
};
