/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __LLIST_H__
#define __LLIST_H__

#include "config.h"

typedef struct lnode lnode_t;
struct lnode {
  lnode_t* next_ptr;
  uint8_t* data_ptr;
};

typedef struct llist llist_t;
struct llist {
  lnode_t* head_ptr;
  lnode_t* tail_ptr;
};

////////////// Linked list - Fonction prototypes //////////////
void  llist_setup();
void  llist_raz(llist_t* llist_ptr);
void  lliste_builde(llist_t* llist_ptr, void* nodes_array_ptr, const int item_count, const int item_size);
void* llist_pop_front(llist_t* llist_ptr);
void  llist_extract_node(llist_t* llist_ptr, void* prevData_ptr, void* data_ptr);
void  llist_push_front(llist_t* llist_ptr, void* data_ptr);
void  llist_push_back(llist_t* llist_ptr, void* data_ptr);
void  llist_swap_llist(llist_t* llistA_ptr, llist_t* llistB_ptr);
void  llist_save_nodes(llist_t* dst_ptr, llist_t* src_ptr);

////////////// Iterators //////////////
#define ITERATOR_START_FROM_HEAD(list) (((llist_t *)(list))->head_ptr)
#define ITERATOR_NEXT(node) (((lnode_t*)(node))->next_ptr)

#endif /*__LLIST_H__*/
