/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "llist.h"

void llist_raz(llist_t* llist_ptr) {
  llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
};

void llist_init(llist_t* llist_ptr, void* nodesArray_ptr, const int item_count, const int item_size){
  uint8_t* item_ptr = (uint8_t*)nodesArray_ptr;
  llist_raz(llist_ptr);
  for (int i = 0; i < item_count; i++) {
    llist_push_front(llist_ptr, item_ptr);
    item_ptr += item_size;
  };
};

void* llist_pop_front(llist_t* llist_ptr) {
  lnode_t* node = llist_ptr->head_ptr;
  if (node != NULL) {
    if (llist_ptr->head_ptr != llist_ptr->tail_ptr) {
      llist_ptr->head_ptr = llist_ptr->head_ptr->next_ptr;
    }
    else {
      llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
    };
    return node;
  }
  else {
    return NULL;
  };
};

void llist_push_front(llist_t* llist_ptr, void* data_ptr) {
  lnode_t* node = (lnode_t*)data_ptr;
  if (llist_ptr->head_ptr != NULL) {
    node->next_ptr = llist_ptr->head_ptr;
    llist_ptr->head_ptr = node;
  }
  else {
    node->next_ptr = NULL;
    llist_ptr->head_ptr = llist_ptr->tail_ptr = node;
  };
};

void llist_push_back(llist_t* llist_ptr, void* data_ptr) {
  lnode_t* node = (lnode_t*)data_ptr;
  node->next_ptr = NULL;
  if (llist_ptr->head_ptr != NULL) {
    llist_ptr->tail_ptr->next_ptr = node;
    llist_ptr->tail_ptr = node;
  }
  else {
    llist_ptr->head_ptr = llist_ptr->tail_ptr = node;
  };
};

// linked-list node extractor
void llist_extract_node(llist_t* llist_ptr, void* prevData_ptr, void* data_ptr) {
  lnode_t* nodeToExtract = (lnode_t*)data_ptr;
  lnode_t* prevNode_ptr = (lnode_t*)prevData_ptr;

  if (llist_ptr->head_ptr == llist_ptr->tail_ptr) {
    llist_ptr->head_ptr = llist_ptr->tail_ptr = NULL;
  }
  else {
    if (nodeToExtract == llist_ptr->head_ptr) {
      llist_ptr->head_ptr = llist_ptr->head_ptr->next_ptr;
    }
    else if (nodeToExtract == llist_ptr->tail_ptr) {
      llist_ptr->tail_ptr = prevNode_ptr;
      prevNode_ptr->next_ptr = NULL;
    }
    else {
      prevNode_ptr->next_ptr = nodeToExtract->next_ptr;
    };
  };
};

void llist_swap_llist(llist_t* llistA_ptr, llist_t* llistB_ptr) {
  if (llistA_ptr->head_ptr != NULL || llistB_ptr->head_ptr != NULL) {
    lnode_t* tmp_head_ptr = (lnode_t*)llistA_ptr->head_ptr;
    lnode_t* tmp_tail_ptr = (lnode_t*)llistA_ptr->tail_ptr;
    llistA_ptr->head_ptr = llistB_ptr->head_ptr;
    llistA_ptr->tail_ptr = llistB_ptr->tail_ptr;
    llistB_ptr->head_ptr = tmp_head_ptr;
    llistB_ptr->tail_ptr = tmp_tail_ptr;
  };
};

void llist_save_nodes(llist_t* dst_ptr, llist_t* src_ptr) {
  if (src_ptr->head_ptr != NULL) {
    if (dst_ptr->head_ptr != NULL) {
      dst_ptr->tail_ptr->next_ptr = src_ptr->head_ptr;
      dst_ptr->tail_ptr = src_ptr->tail_ptr;
      src_ptr->tail_ptr = src_ptr->head_ptr = NULL;
    }
    else {
      dst_ptr->head_ptr = src_ptr->head_ptr;
      dst_ptr->tail_ptr = src_ptr->tail_ptr;
      src_ptr->tail_ptr = src_ptr->head_ptr = NULL;
    };
  };
};
