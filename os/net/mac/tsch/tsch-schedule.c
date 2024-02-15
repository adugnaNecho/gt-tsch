/*
 * Copyright (c) 2014, SICS Swedish ICT.
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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         IEEE 802.15.4 TSCH MAC schedule manager.
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 *         Beshr Al Nahas <beshr@sics.se>
 */

/**
 * \addtogroup tsch
 * @{
*/

#include "contiki.h"
#include "dev/leds.h"
#include "lib/memb.h"
#include "net/nbr-table.h"
#include "net/packetbuf.h"
#include "net/queuebuf.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/framer/frame802154.h"
#include "sys/process.h"
#include "sys/node-id.h"
#include "sys/rtimer.h"
#include <string.h>
#include "lib/assert.h"
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "TSCH Sched"
#define LOG_LEVEL LOG_LEVEL_MAC

/* Pre-allocated space for links */
MEMB(link_memb, struct tsch_link, TSCH_SCHEDULE_MAX_LINKS);
/* Pre-allocated space for slotframes */
MEMB(slotframe_memb, struct tsch_slotframe, TSCH_SCHEDULE_MAX_SLOTFRAMES);
/* List of slotframes (each slotframe holds its own list of links) */
LIST(slotframe_list);

/* Adds and returns a slotframe (NULL if failure) */
struct tsch_slotframe *
tsch_schedule_add_slotframe(uint16_t handle, uint16_t size)
{
    
  if(size == 0) {
    return NULL;
  }

  if(tsch_schedule_get_slotframe_by_handle(handle)) {
    /* A slotframe with this handle already exists */
    return NULL;
  }

  if(tsch_get_lock()) {
    struct tsch_slotframe *sf = memb_alloc(&slotframe_memb);
    if(sf != NULL) {
      /* Initialize the slotframe */
      sf->handle = handle;
      TSCH_ASN_DIVISOR_INIT(sf->size, size);
      LIST_STRUCT_INIT(sf, links_list);
      /* Add the slotframe to the global list */
      list_add(slotframe_list, sf);
    }
    LOG_INFO("add_slotframe %u %u\n",
           handle, size);
    tsch_release_lock();
 // printf("omid: add a slotframe\n");
    return sf;
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
/* Removes all slotframes, resulting in an empty schedule */
int
tsch_schedule_remove_all_slotframes(void)
{
  struct tsch_slotframe *sf;
  while((sf = list_head(slotframe_list))) {
    if(tsch_schedule_remove_slotframe(sf) == 0) {
      return 0;
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
/* Removes a slotframe Return 1 if success, 0 if failure */
int
tsch_schedule_remove_slotframe(struct tsch_slotframe *slotframe)
{
  if(slotframe != NULL) {
    /* Remove all links belonging to this slotframe */
    struct tsch_link *l;
    while((l = list_head(slotframe->links_list))) {
      tsch_schedule_remove_link(slotframe, l);
    }

    /* Now that the slotframe has no links, remove it. */
    if(tsch_get_lock()) {
      LOG_INFO("remove slotframe %u %u\n", slotframe->handle, slotframe->size.val);
      memb_free(&slotframe_memb, slotframe);
      list_remove(slotframe_list, slotframe);
      printf("omid: remove a slotframe\n");
      tsch_release_lock();
      return 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/* Looks for a slotframe from a handle */
struct tsch_slotframe *
tsch_schedule_get_slotframe_by_handle(uint16_t handle)
{
  if(!tsch_is_locked()) {
    struct tsch_slotframe *sf = list_head(slotframe_list);
    while(sf != NULL) {
      if(sf->handle == handle) {
        return sf;
      }
      sf = list_item_next(sf);
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
/* Looks for a link from a handle */
struct tsch_link *
tsch_schedule_get_link_by_handle(uint16_t handle)
{
  if(!tsch_is_locked()) {
    struct tsch_slotframe *sf = list_head(slotframe_list);
    while(sf != NULL) {
      struct tsch_link *l = list_head(sf->links_list);
      /* Loop over all items. Assume there is max one link per timeslot */
      while(l != NULL) {
        if(l->handle == handle) {
          return l;
        }
        l = list_item_next(l);
      }
      sf = list_item_next(sf);
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static const char *
print_link_options(uint16_t link_options)
{
  static char buffer[20];
  unsigned length;

  buffer[0] = '\0';
  if(link_options & LINK_OPTION_TX) {
    strcat(buffer, "Tx|");
  }
  if(link_options & LINK_OPTION_RX) {
    strcat(buffer, "Rx|");
  }
  if(link_options & LINK_OPTION_SHARED) {
    strcat(buffer, "Sh|");
  }
  length = strlen(buffer);
  if(length > 0) {
    buffer[length - 1] = '\0';
  }

  return buffer;
}
/*---------------------------------------------------------------------------*/
static const char *
print_link_type(uint16_t link_type)
{
  switch(link_type) {
  case LINK_TYPE_NORMAL:
    return "NORMAL";
  case LINK_TYPE_ADVERTISING:
    return "ADV";
  case LINK_TYPE_ADVERTISING_ONLY:
    return "ADV_ONLY";
  default:
    return "?";
  }
}
/*---------------------------------------------------------------------------*/
/* Adds a link to a slotframe, return a pointer to it (NULL if failure) */


int dtsf_check_TX_timeslot(struct tsch_slotframe *slotframe, uint16_t timeslot_offset, uint16_t channel_offset)
{
    struct tsch_link* link = tsch_schedule_get_link_by_timeslot(slotframe, timeslot_offset, channel_offset);
    if(link==NULL)
    {
       // printf("return 0  ch=%d   ts=%d\n",(int)channel_offset,(int)timeslot_offset);
        return 0;
    }
      
     // const linkaddr_t* test=rpl_neighbor_get_lladdr(curr_instance.dag.preferred_parent);
        struct tsch_neighbor* time_source= tsch_queue_get_time_source();
        const linkaddr_t* test= &time_source->addr;

      
      
  //assert(test != NULL);
  if(test==NULL)
  {
      return 0;
  }
  struct tsch_neighbor *ts = tsch_queue_get_nbr(test);
    if(link->link_type!=LINK_TYPE_NORMAL  ||  link->link_options!=LINK_OPTION_TX || linkaddr_cmp(&link->addr,&ts->addr)==0 ||  link->reserved==1 )
    {
        if(link->link_type!=LINK_TYPE_NORMAL)
        {
           // printf("1\n");
        }
         if(link->link_options!=LINK_OPTION_TX)
        {
          //  printf("2\n");
        }
         if(linkaddr_cmp(&link->addr,&ts->addr)==0)
        {
           // printf("3\n");
        }
         if(link->reserved==1)
        {
           // printf("4\n");
        }
        //printf("Error: return problem\n");
        return 0;
    }
    
  //  printf("return 1\n");
    return 1;
    
}


int dtsf_check_RX_timeslot(struct tsch_slotframe *slotframe, uint16_t timeslot_offset, uint16_t channel_offset)
{
		struct tsch_link* link = tsch_schedule_get_link_by_timeslot(slotframe, timeslot_offset, channel_offset);
		if(link==NULL)
		{
		   // printf("return 0  ch=%d   ts=%d\n",(int)channel_offset,(int)timeslot_offset);
			return 0;
		}
      
  
    if(link->link_type!=LINK_TYPE_NORMAL  ||  link->link_options!=LINK_OPTION_RX )
    {
        if(link->link_type!=LINK_TYPE_NORMAL)
        {
           // printf("1\n");
        }
         if(link->link_options!=LINK_OPTION_RX)
        {
          //  printf("2\n");
        }
        
        return 0;
    }
    
    
  //  printf("return 1\n");
    return 1;
    
}


struct tsch_link *
tsch_schedule_add_link(struct tsch_slotframe *slotframe,
                       uint8_t link_options, enum link_type link_type, const linkaddr_t *address,
                       uint16_t timeslot, uint16_t channel_offset)
	{
					// printf("test13\n");
				  struct tsch_link *l = NULL;
				  if(slotframe != NULL)
					{
						 // printf("test3\n");

					/* We currently support only one link per timeslot in a given slotframe. */

					/* Validation of specified timeslot and channel_offset */
								if(timeslot > (slotframe->size.val - 1)) 
								{
												printf("! add_link invalid timeslot: %u\n", timeslot);
												// printf("test22\n");
												return NULL;
								}
    
								if( tsch_schedule_get_link_by_just_timeslot(slotframe, timeslot) != NULL )
								{
											printf("ERRORRRRRRRRRRRRRRRRRRRR   %d\n",timeslot);
											return NULL;
								}
							  //  printf("test2\n");

								/* Start with removing the link currently installed at this timeslot (needed
								 * to keep neighbor state in sync with link options etc.) */
							tsch_schedule_remove_link_by_timeslot(slotframe, timeslot, channel_offset);
							if(!tsch_get_lock())
							{
										printf("! add_link memb_alloc couldn't take lock\n");
							} 
							else 
							{
                              //  	printf("test111\n");
                                	//printf("test111\n");
										l = memb_alloc(&link_memb);
										if(l == NULL) 
										{
													printf("! add_link memb_alloc failed\n");
													tsch_release_lock();
										} 
									  else
									  {
                                            	//printf("test110\n");
												static int current_link_handle = 0;
												struct tsch_neighbor *n;
											/* Add the link to the slotframe */
												list_add(slotframe->links_list, l);
											/* Initialize link */
												l->handle = current_link_handle++;
												l->link_options = link_options;
										  
												l->link_type = link_type;

												l->slotframe_handle = slotframe->handle;
												l->timeslot = timeslot;
												l->channel_offset = channel_offset;
												l->data = NULL;
												if(address == NULL) 
												{
														address = &linkaddr_null;
												}
												linkaddr_copy(&l->addr, address);
												printf("add a link : type=%s sf=%u opt=%s  ts=%u ch=%d\n",print_link_type(link_type), slotframe->handle,print_link_options(link_options), timeslot, channel_offset);
												LOG_INFO("add_link sf=%u opt=%s type=%s ts=%u ch=%u addr=", slotframe->handle,
												print_link_options(link_options), print_link_type(link_type), timeslot, channel_offset);
												LOG_INFO_LLADDR(address);
												LOG_INFO_("\n");
												/* Release the lock before we update the neighbor (will take the lock) */
												tsch_release_lock();
												//  tsch_schedule_print();
												if(l->link_options==LINK_OPTION_TX && l->link_type==LINK_TYPE_NORMAL)
												{
																if(required_slots>0)
																{
																		required_slots = required_slots - 1;
															
																}

										
																if(allocate_slot_for_packet_generation==0)
																{
																				l->reserved=1;
																				current_number_slots_for_packet_generation++;
																				current_packet_generation_rate = convert_slots_to_rate(current_number_slots_for_packet_generation);
																				//printf("current number of slots = %d\n",current_number_slots_for_packet_generation);
																				if(current_packet_generation_rate == packet_generation_rate)
																				{
																						allocate_slot_for_packet_generation= 1;
																				}
														   
																}
																else
																{
																		free_uplink_timeslots= free_uplink_timeslots + 1;
																}
												}
									
												if(l->link_options==LINK_OPTION_RX && l->link_type==LINK_TYPE_NORMAL)
												{
																struct tsch_neighbor *n = tsch_queue_get_nbr(address);
																if(n != NULL) 
																{
																
																		n->rx_links_count ++;
																		// printf("link count=%d\n", n->rx_links_count);
																}
									
																if(!tsch_is_coordinator)
																{
																			if(dtsf_check_TX_timeslot(slotframe, timeslot+1, parent_channel) == 1)
																			{
																					if( l->reserved==0)
																					{
																							l->reserved=1;
																							free_uplink_timeslots= free_uplink_timeslots - 1;
																					}
																			
																			}
																			else
																			{
																						if(    ((timeslot+1)%5)==0 &&  dtsf_check_TX_timeslot(slotframe, timeslot+2, parent_channel) == 1)
																						{
																								if( l->reserved==0)
																								{
																										l->reserved=1;
																										free_uplink_timeslots= free_uplink_timeslots - 1;
																								}
																								
																								
																						}
																						else
																						{
																							printf("error in setting the reserved bit  channel=%d   timeslot=%d\n",(int)parent_channel,(int)timeslot+1);

																						}
																		  // tsch_schedule_print();
																			}
											
																}
																else
																{
																	
																		l->reserved=1;
																		free_uplink_timeslots= free_uplink_timeslots - 1;
																}
										
												}
									
									
											if(l->link_options == LINK_OPTION_TX) 
											{
														n = tsch_queue_add_nbr(&l->addr);
								   
														if(n != NULL) 
														{
																	n->tx_links_count++;
																	if(!(l->link_options & LINK_OPTION_SHARED))
																	 {
																			n->dedicated_tx_links_count++;
																	}
														}
											}
									
									
							}
					}
			}
  return l;
}



int
tsch_schedule_delete_link(struct tsch_slotframe *slotframe,
                       uint8_t link_options, enum link_type link_type, const linkaddr_t *address,
                       uint16_t timeslot, uint16_t channel_offset)
{
   // printf("test1\n");
  struct tsch_link *l = NULL;
  if(slotframe != NULL) 
  {

    if(timeslot > (slotframe->size.val - 1)) 
    {
      printf("! delete_link invalid timeslot: %u\n", timeslot);
      
      return -1;
    }



        if(l->link_options==LINK_OPTION_TX && l->link_type==LINK_TYPE_NORMAL)
        {
                  l=tsch_schedule_get_link_by_timeslot(slotframe, timeslot, channel_offset);
           
                   if(l!=NULL)
                   {
                                if(l->reserved==0)
                                {
                                   if(free_uplink_timeslots>0)
                                   {
                                                              free_uplink_timeslots = free_uplink_timeslots - 1;
                                   }
                                  else
                                  {
                                      printf("error in free_uplink_timeslots\n");
                                  }
                                   
                               }
                               else
                               {
                                   required_slots = required_slots + 1;
                               }
                     
         
                       
                   }
                   else
                   {
                       printf("cannot find uplink for deleting\n");
                   }
        
           
            
               
        
          
        }
        
        

        if(l->link_options==LINK_OPTION_RX && l->link_type==LINK_TYPE_NORMAL)
        {
              struct tsch_neighbor *n = tsch_queue_get_nbr(address);
             if(n != NULL) 
             {
                    if( n->rx_links_count>0)
                    {
                        n->rx_links_count = n->rx_links_count -1;
                       // rx_links_count
                        
                    printf("link count=%d\n", n->rx_links_count);
                    }
                    else
                    {
                        printf("error in decreasing the link count\n");
                    }
                    
             }
        
            if(!tsch_is_coordinator)
            {
                             l=tsch_schedule_get_link_by_timeslot(slotframe, timeslot+1, parent_channel);
                             if(l!=NULL)
                             {
                                 if( l->reserved==1)
                                 {
                                     l->reserved=0;
                                     
                                        free_uplink_timeslots = free_uplink_timeslots + 1;
                                        
                                     
                                 }
                                 
                             }
                
            }
           
           
           
           
            
            if(tsch_is_coordinator)
            {
                free_uplink_timeslots = free_uplink_timeslots + 1;
                l->reserved = 0;
            }
          
          
        }
        
        
       
        
        
     
      
    
        
    l=tsch_schedule_get_link_by_just_timeslot(slotframe, timeslot);  
        

    if(tsch_get_lock()) 
    {
          uint8_t link_options;
          linkaddr_t addr;

          /* Save link option and addr in local variables as we need them
           * after freeing the link */
          link_options = l->link_options;
          linkaddr_copy(&addr, &l->addr);

          /* The link to be removed is scheduled as next, set it to NULL
           * to abort the next link operation */
          if(l == current_link) 
          {
            current_link = NULL;
          }

          list_remove(slotframe->links_list, l);
          memb_free(&link_memb, l);
          tsch_release_lock();  
       
          if(l->link_options & LINK_OPTION_TX) 
          {
            struct tsch_neighbor *n = tsch_queue_get_nbr(&addr);
            if(n != NULL) 
            {
              n->tx_links_count--;
              if(!(link_options & LINK_OPTION_SHARED)) 
              {
                n->dedicated_tx_links_count--;
              }
            }
          }

          return 1;
    } 
    else 
    {
      LOG_ERR("! remove_link memb_alloc couldn't take lock\n");
      return -1;
    }
    
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
/* Removes a link from slotframe. Return 1 if success, 0 if failure */
int
tsch_schedule_remove_link(struct tsch_slotframe *slotframe, struct tsch_link *l)
{
  if(slotframe != NULL && l != NULL && l->slotframe_handle == slotframe->handle) {
    if(tsch_get_lock()) {
      uint8_t link_options;
      linkaddr_t addr;

      /* Save link option and addr in local variables as we need them
       * after freeing the link */
      link_options = l->link_options;
      linkaddr_copy(&addr, &l->addr);

      /* The link to be removed is scheduled as next, set it to NULL
       * to abort the next link operation */
      if(l == current_link) {
        current_link = NULL;
      }
      LOG_INFO("remove_link sf=%u opt=%s type=%s ts=%u ch=%u addr=",
               slotframe->handle,
               print_link_options(l->link_options),
               print_link_type(l->link_type), l->timeslot, l->channel_offset);
      LOG_INFO_LLADDR(&l->addr);
      LOG_INFO_("\n");

      list_remove(slotframe->links_list, l);
      memb_free(&link_memb, l);

      /* Release the lock before we update the neighbor (will take the lock) */
      tsch_release_lock();

      /* This was a tx link to this neighbor, update counters */
      if(link_options & LINK_OPTION_TX) {
        struct tsch_neighbor *n = tsch_queue_get_nbr(&addr);
        if(n != NULL) {
          n->tx_links_count--;
          if(!(link_options & LINK_OPTION_SHARED)) {
            n->dedicated_tx_links_count--;
          }
        }
      }
        
      
      
      
      return 1;
    } else {
      LOG_ERR("! remove_link memb_alloc couldn't take lock\n");
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/* Removes a link from slotframe and timeslot. Return a 1 if success, 0 if failure */
int
tsch_schedule_remove_link_by_timeslot(struct tsch_slotframe *slotframe,
                                      uint16_t timeslot, uint16_t channel_offset)
{
  return tsch_schedule_remove_link(slotframe,
                                   tsch_schedule_get_link_by_timeslot(slotframe, timeslot, channel_offset));
}
/*---------------------------------------------------------------------------*/
/* Looks within a slotframe for a link with a given timeslot */



//dtsf/////find a free timeslot for adv link//////////////////////////

struct tsch_link*
tsch_schedule_get_link_by_just_timeslot(struct tsch_slotframe *slotframe,
                                   uint16_t timeslot)
{
   
  if(!tsch_is_locked()) {
    if(slotframe != NULL) {
      struct tsch_link *l = list_head(slotframe->links_list);
      /* Loop over all items. Assume there is max one link per timeslot */
      while(l != NULL) {
        if(l->timeslot == timeslot) {
          return l;
        }
        l = list_item_next(l);
      }
      return l;
    }
  }
  
  return NULL;
}



int dtsf_find_free_adv_slot(struct tsch_slotframe *slotframe, uint16_t channel_offset)
{
    //printf("looking for free slot in channel %d\n",channel_offset); 
    uint16_t time_offset=0;
    while(time_offset < TSCH_SCHEDULE_DEFAULT_LENGTH)
    {
        if(tsch_schedule_get_link_by_just_timeslot(slotframe, time_offset) == NULL)
        {
             if(tsch_schedule_get_link_by_timeslot(slotframe, time_offset, channel_offset) == NULL)
                     {
                       //  printf("find it timeslot=%d channel=%d\n",time_offset,channel_offset);
                         return time_offset;
                     }
            
        }
        time_offset++;
        
    }
    printf("Error, cannot find free timeslot\n");
    return -1;
    
}

int dtsf_check_consequent_RX_timeslot(struct tsch_slotframe *slotframe, uint16_t timeslot_offset, uint16_t channel_offset, const linkaddr_t *peer_addr)
{
    struct tsch_link* link1 = tsch_schedule_get_link_by_timeslot(slotframe, timeslot_offset+1, channel_offset);
    struct tsch_link* link2 = tsch_schedule_get_link_by_timeslot(slotframe, timeslot_offset-1, channel_offset);
    struct tsch_link* link3 = tsch_schedule_get_link_by_timeslot(slotframe, timeslot_offset-2, channel_offset);
    int check1=0;
    int check2=0;
    int check3=0;
    
    if(link1 == NULL)
    {
         check1=1;
    }
    else 
    { 
        if (linkaddr_cmp(&link1->addr,peer_addr)<1) 
        {
            check1=1;
        }
    }
    
    if(link2 == NULL)
    {
         check2=1;
    }
    else 
    { 
        if (linkaddr_cmp(&link2->addr,peer_addr)<1) 
        {
            check2=1;
        }
    }
    
    /////GT-TSCH/////////////////////////
    if(  (timeslot_offset-1)%5  != 0)
    {
        check3=1;
    }
    else
    {
        if( linkaddr_cmp(&link3->addr,peer_addr)<1)
        {
            check3=1;
        }
    }
    ////////////////////////////////////////////////////////
 
    
    if(check1==1 && check2==1 && check3==1)
    {
        return 1;
    }

    return 0;
}


int dtsf_find_last_uplinks( const linkaddr_t *peer_addr, uint16_t number_of_links, sf_simple_cell_t* cell_list)
{
    
    uint16_t time_offset=0;
    uint16_t allocated=0;
    struct tsch_slotframe *slotframe;
    slotframe = tsch_schedule_get_slotframe_by_handle(0);
    time_offset=TSCH_SCHEDULE_CONF_DEFAULT_LENGTH-1;
    uint16_t channel_offset=children_channel;
    while(time_offset > 0 && allocated<number_of_links)
    {
        if(tsch_schedule_get_link_by_just_timeslot(slotframe, time_offset) != NULL)
        {
            struct tsch_link* link= tsch_schedule_get_link_by_just_timeslot(slotframe, time_offset); 
            
             if(linkaddr_cmp(&link->addr,peer_addr))
             {
                  
                                cell_list[allocated].channel_offset=channel_offset;
                                cell_list[allocated].timeslot_offset=time_offset;
                                allocated++;
             }
        }
        time_offset = time_offset - 1;
    }
    
    return allocated;
}

int dtsf_find_free_uplink_slot(struct tsch_slotframe *slotframe, uint16_t channel_offset, uint16_t number, sf_simple_cell_t* cell_list, const linkaddr_t *peer_addr)
{
    //printf("looking for free uplink slot in channel=%d number=%d\n",channel_offset,number); 
    
    uint16_t time_offset=1;
    uint16_t allocated=0;
    
   // uint16_t limit= (free_uplink_timeslots/maximum_number_of_children);
      uint16_t limit= 0;

    //if(limit>4)
   // {
      //  limit=4;
    //}
 //uint16_t limit= number + 3;
// limit=0;
  //  printf("number=%d   limit=%d  free_uplink=%d  maximum_number=%d\n",(int)number,(int)limit,(int)free_uplink_timeslots,(int)maximum_number_of_children);
    while(time_offset < TSCH_SCHEDULE_CONF_DEFAULT_LENGTH && allocated < number+limit)
    {
        if(tsch_schedule_get_link_by_just_timeslot(slotframe, time_offset) == NULL)
        {
             if(tsch_schedule_get_link_by_timeslot(slotframe, time_offset, channel_offset) == NULL)
             {
                   if(tsch_is_coordinator)
                   {
                        if(dtsf_check_consequent_RX_timeslot(slotframe, time_offset, channel_offset, peer_addr)==1)
                        {
                           // if(find_in_cell_list_reserved(time_offset,channel_offset)==0)
                           // {
                           // printf("find downlink slot in coordinator  channel=%d  time=%d\n",channel_offset,time_offset);
                                cell_list[allocated].channel_offset=channel_offset;
                                cell_list[allocated].timeslot_offset=time_offset;
                              //  add_to_cell_list_reserved(time_offset,channel_offset);
                                allocated++;
                                time_offset++;
                           // }
                        }
                        
                   }
                   else
                   {
                        // printf("finding channel=%d  time=%d\n",parent_channel,time_offset+1);
							   if(dtsf_check_TX_timeslot(slotframe, time_offset+1, parent_channel) == 1)
							   {
								  // if(find_in_cell_list_reserved(time_offset,channel_offset)==0)
								  //  {
								  // printf("find downlink slot in non coordinator  channel=%d  time=%d\n",channel_offset,time_offset);
										cell_list[allocated].channel_offset=channel_offset;
										cell_list[allocated].timeslot_offset=time_offset;
									   //add_to_cell_list_reserved(time_offset,channel_offset);
										allocated++;
										time_offset++;
								   // }
								
							   }
							   else{
											if( ((time_offset+1)%5) == 0  &&  dtsf_check_TX_timeslot(slotframe, time_offset+2, parent_channel) == 1)
											{
												cell_list[allocated].channel_offset=channel_offset;
												cell_list[allocated].timeslot_offset=time_offset;
									   //add_to_cell_list_reserved(time_offset,channel_offset);
												allocated++;
												time_offset++;
												time_offset++;
											}
										}
						
							   
					   
					   
                   }
                 
                         
             }
            
        }
        time_offset++;
    }
    
    return allocated;
    
}


int dtsf_find_free_adv_link_slot(struct tsch_slotframe *slotframe, uint16_t channel_offset, uint16_t number, sf_simple_cell_t* cell_list, const linkaddr_t *peer_addr)
{
   // printf("looking for free adv link slot in channel=%d number=%d\n",channel_offset,number); 
   

    uint16_t time_offset=0;
    uint16_t allocated=0;
        
    while(time_offset < TSCH_SCHEDULE_CONF_DEFAULT_LENGTH && allocated < number)
    {
        if(tsch_schedule_get_link_by_just_timeslot(slotframe, time_offset) == NULL)
        {
             if(tsch_schedule_get_link_by_timeslot(slotframe, time_offset, channel_offset) == NULL)
             {
                   if(find_in_cell_list_reserved(time_offset,channel_offset)==0)
                   {
                           // printf("find adv slot in  channel=%d  time=%d\n",channel_offset,time_offset);
                            cell_list[allocated].channel_offset=channel_offset;
                            cell_list[allocated].timeslot_offset=time_offset;
                            add_to_cell_list_reserved(time_offset,channel_offset);
                            allocated++;
                            
                   }
             }
            
        }
        time_offset++;
    }
    
    return allocated;
    
}




////////////////////////////////////////////////////////////////////





struct tsch_link*
tsch_schedule_get_link_by_timeslot(struct tsch_slotframe *slotframe,
                                   uint16_t timeslot, uint16_t channel_offset)
{
  if(!tsch_is_locked()) {
    if(slotframe != NULL) {
      struct tsch_link *l = list_head(slotframe->links_list);
      /* Loop over all items. Assume there is max one link per timeslot */
      while(l != NULL) {
        if(l->timeslot == timeslot && l->channel_offset == channel_offset) {
          return l;
        }
        l = list_item_next(l);
      }
      return l;
    }
  }
  else
  {
        printf("tsch is locked\n");

  }
  return NULL;
}



/*---------------------------------------------------------------------------*/
static struct tsch_link *
default_tsch_link_comparator(struct tsch_link *a, struct tsch_link *b)
{
  if(!(a->link_options & LINK_OPTION_TX)) {
    /* None of the links are Tx: simply return the first link */
    return a;
  }

  /* Two Tx links at the same slotframe; return the one with most packets to send */
  if(!linkaddr_cmp(&a->addr, &b->addr)) {
    struct tsch_neighbor *an = tsch_queue_get_nbr(&a->addr);
    struct tsch_neighbor *bn = tsch_queue_get_nbr(&b->addr);
    int a_packet_count = an ? ringbufindex_elements(&an->tx_ringbuf) : 0;
    int b_packet_count = bn ? ringbufindex_elements(&bn->tx_ringbuf) : 0;
    /* Compare the number of packets in the queue */
    return a_packet_count >= b_packet_count ? a : b;
  }

  /* Same neighbor address; simply return the first link */
  return a;
}

/*---------------------------------------------------------------------------*/
/* Returns the next active link after a given ASN, and a backup link (for the same ASN, with Rx flag) */
struct tsch_link *
tsch_schedule_get_next_active_link(struct tsch_asn_t *asn, uint16_t *time_offset,
    struct tsch_link **backup_link)
{
  uint16_t time_to_curr_best = 0;
  struct tsch_link *curr_best = NULL;
  struct tsch_link *curr_backup = NULL; /* Keep a back link in case the current link
  turns out useless when the time comes. For instance, for a Tx-only link, if there is
  no outgoing packet in queue. In that case, run the backup link instead. The backup link
  must have Rx flag set. */
  if(!tsch_is_locked()) {
    struct tsch_slotframe *sf = list_head(slotframe_list);
    /* For each slotframe, look for the earliest occurring link */
    while(sf != NULL) {
      /* Get timeslot from ASN, given the slotframe length */
      uint16_t timeslot = TSCH_ASN_MOD(*asn, sf->size);
      struct tsch_link *l = list_head(sf->links_list);
      while(l != NULL) {
        uint16_t time_to_timeslot =
          l->timeslot > timeslot ?
          l->timeslot - timeslot :
          sf->size.val + l->timeslot - timeslot;
        if(curr_best == NULL || time_to_timeslot < time_to_curr_best) {
          time_to_curr_best = time_to_timeslot;
          curr_best = l;
          curr_backup = NULL;
        } else if(time_to_timeslot == time_to_curr_best) {
          struct tsch_link *new_best = NULL;
          /* Two links are overlapping, we need to select one of them.
           * By standard: prioritize Tx links first, second by lowest handle */
          if((curr_best->link_options & LINK_OPTION_TX) == (l->link_options & LINK_OPTION_TX)) {
            /* Both or neither links have Tx, select the one with lowest handle */
            if(l->slotframe_handle != curr_best->slotframe_handle) {
              if(l->slotframe_handle < curr_best->slotframe_handle) {
                new_best = l;
              }
            } else {
              /* compare the link against the current best link and return the newly selected one */
              new_best = TSCH_LINK_COMPARATOR(curr_best, l);
            }
          } else {
            /* Select the link that has the Tx option */
            if(l->link_options & LINK_OPTION_TX) {
              new_best = l;
            }
          }

          /* Maintain backup_link */
          if(curr_backup == NULL) {
            /* Check if 'l' best can be used as backup */
            if(new_best != l && (l->link_options & LINK_OPTION_RX)) { /* Does 'l' have Rx flag? */
              curr_backup = l;
            }
            /* Check if curr_best can be used as backup */
            if(new_best != curr_best && (curr_best->link_options & LINK_OPTION_RX)) { /* Does curr_best have Rx flag? */
              curr_backup = curr_best;
            }
          }

          /* Maintain curr_best */
          if(new_best != NULL) {
            curr_best = new_best;
          }
        }

        l = list_item_next(l);
      }
      sf = list_item_next(sf);
    }
    if(time_offset != NULL) {
      *time_offset = time_to_curr_best;
    }
  }
  if(backup_link != NULL) {
    *backup_link = curr_backup;
  }
  return curr_best;
}
/*---------------------------------------------------------------------------*/
/* Module initialization, call only once at startup. Returns 1 is success, 0 if failure. */
int
tsch_schedule_init(void)
{
  if(tsch_get_lock()) {
    memb_init(&link_memb);
    memb_init(&slotframe_memb);
    list_init(slotframe_list);
    tsch_release_lock();
    return 1;
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
/* Create a 6TiSCH minimal schedule */
void
tsch_schedule_create_minimal(void)
{
    printf("omid: minimal schedule created\n");
  
  /* First, empty current schedule */
  tsch_schedule_remove_all_slotframes();
  struct tsch_slotframe *sf_min;
  
         sf_min = tsch_schedule_add_slotframe(0, TSCH_SCHEDULE_DEFAULT_LENGTH);
                //tsch_schedule_add_slotframe(0, TSCH_SCHEDULE_DEFAULT_LENGTH);

  /* Build 6TiSCH minimal schedule.
   * We pick a slotframe length of TSCH_SCHEDULE_DEFAULT_LENGTH */
   // if(node_id==1)
  //{
      
  /* Add a single Tx|Rx|Shared slot using broadcast address (i.e. usable for unicast and broadcast).
   * We set the link type to advertising, which is not compliant with 6TiSCH minimal schedule
   * but is required according to 802.15.4e if also used for EB transmission.
   * 
   * Timeslot: 0, channel offset: 0. */
   int n=7;
   int i=0;
   for(i=0;i<TSCH_SCHEDULE_CONF_DEFAULT_LENGTH;i=i+5)
   {
           tsch_schedule_add_link(sf_min, (LINK_OPTION_RX | LINK_OPTION_TX | LINK_OPTION_SHARED), LINK_TYPE_ADVERTISING, &tsch_broadcast_address,i, default_channel);
		 
   }



 if(tsch_is_coordinator)
   {
 
       
       free_uplink_timeslots=free_uplink_timeslots - n;
       find_shared_timeslot_children();
                                          
   }     

 
          

 
}
/*---------------------------------------------------------------------------*/
struct tsch_slotframe *
tsch_schedule_slotframe_head(void)
{
  return list_head(slotframe_list);
}
/*---------------------------------------------------------------------------*/
struct tsch_slotframe *
tsch_schedule_slotframe_next(struct tsch_slotframe *sf)
{
  return list_item_next(sf);
}
/*---------------------------------------------------------------------------*/
/* Prints out the current schedule (all slotframes and links) */
void
tsch_schedule_print(void)
{
   // printf("here\n");
    if(!tsch_is_locked()) {
    struct tsch_slotframe *sf = list_head(slotframe_list);

    printf("----- start slotframe list -----\n");

    while(sf != NULL) {
      struct tsch_link *l = list_head(sf->links_list);

      printf("Slotframe Handle %u, size %u\n", sf->handle, sf->size.val);

      while(l != NULL) {
        printf("* Link Options %02x, type %u, timeslot %u, channel offset %u, address %u\n",
               l->link_options, l->link_type, l->timeslot, l->channel_offset, l->addr.u8[7]);
        l = list_item_next(l);
      }

      sf = list_item_next(sf);
    }

    printf("----- end slotframe list -----\n");
    }
}
/*---------------------------------------------------------------------------*/
/** @} */
