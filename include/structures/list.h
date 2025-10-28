#ifndef _STRUCTURES_LIST_H_
#define _STRUCTURES_LIST_H_


#include <stdint.h>


struct list_node {
    void *data;
    struct list_node *next;
    struct list_node *prev;
};


struct list {
    struct list_node *head;
    uint32_t size;
};


enum list_status {
    LIST_STATUS_SUCCESS = 0,
    LIST_STATUS_EMPTY_LIST,
    LIST_STATUS_INVALID_INDEX,
    LIST_STATUS_INVALID_ARGUMENT
};


typedef void (*list_node_free_callback)(void *data);


struct list *create_list();


void destroy_list(struct list *list, list_node_free_callback node_free_callback);


void list_default_node_free_callback(void *data);


enum list_status list_add_at(struct list *list, uint32_t index, void *data);


enum list_status list_add(struct list *list, void *data);


enum list_status list_peek_at(struct list *list, uint32_t index, void **data);


enum list_status list_pop_at(struct list *list, uint32_t index, void **data);


enum list_status list_pop_front(struct list *list, void **data);


#endif  // _STRUCTURES_LIST_H_
