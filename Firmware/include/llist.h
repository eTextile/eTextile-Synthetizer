/*
  This file is part of the eTextile-Synthesizer project - https://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __LLIST_H__
#define __LLIST_H__

#include "config.h"

typedef struct lnode_s lnode_t;
struct lnode_s {
  lnode_t* next_ptr;
  void* data_ptr;
};

typedef struct llist_s llist_t;
struct llist_s {
  lnode_t* head_ptr;
  lnode_t* tail_ptr;
};

////////////// Linked list - Fonction prototypes //////////////
void llist_setup(void);
void llist_raz(llist_t* llist_ptr);
void llist_builder(llist_t* llist_ptr, void* nodes_array_ptr, const int item_count, const int item_size);
void* llist_pop_front(llist_t* llist_ptr);
void llist_push_front(llist_t* llist_ptr, void* data_ptr);
void llist_push_back(llist_t* llist_ptr, void* data_ptr);
void llist_swap_llist(llist_t* llistA_ptr, llist_t* llistB_ptr);
void llist_concat_nodes(llist_t* dst_ptr, llist_t* src_ptr);

typedef bool llist_compare_func_t (void*, void*);
void* llist_find_node(llist_t* llist_ptr, void* data_ptr, llist_compare_func_t* func_ptr);

////////////// Iterators //////////////
#define ITERATOR_START_FROM_HEAD(llist) (((llist_t *)(llist))->head_ptr)
#define ITERATOR_NEXT(node) (((lnode_t*)(node))->next_ptr)
#define ITERATOR_DATA(node) (((lnode_t*)(node))->data_ptr)

#endif /*__LLIST_H__*/
