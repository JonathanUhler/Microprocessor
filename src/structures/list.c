#include "structures/list.h"
#include <stdint.h>
#include <stdlib.h>


#include <stdio.h>


struct list *create_list() {
    struct list *list = (struct list *) malloc(sizeof(struct list));

    list->head = NULL;
    list->size = 0;

    return list;
}


void destroy_list(struct list *list, list_node_free_callback node_free_callback) {
    if (list == NULL) {
        return;
    }

    struct list_node *curr = list->head;
    while (curr != NULL) {
        struct list_node *next = curr->next;
        if (node_free_callback != NULL) {
            node_free_callback(curr->data);
        }
        free(curr);
        curr = next;
    }

    free(list);
}


void list_default_node_free_callback(void *data) {
    if (data == NULL) {
        return;
    }
    free(data);
}


enum list_status list_add_at(struct list *list, uint32_t index, void *data) {
    if (list == NULL) {
        return LIST_STATUS_INVALID_ARGUMENT;
    }
    if (index > list->size) {
        return LIST_STATUS_INVALID_INDEX;
    }

    struct list_node *new_node = (struct list_node *) malloc(sizeof(struct list_node));
    new_node->data = data;

    if (index == 0) {
        new_node->prev = NULL;
        new_node->next = list->head;
        if (list->head != NULL) {
            list->head->prev = new_node;
        }
        list->head = new_node;
    }
    else {
        struct list_node *prev = NULL;
        struct list_node *curr = list->head;
        for (uint32_t i = 0; i < index; i++) {
            prev = curr;
            curr = curr->next;
        }

        new_node->next = curr;
        new_node->prev = prev;
        prev->next = new_node;
        if (curr != NULL) {
            curr->prev = new_node;
        }
    }

    list->size++;
    return LIST_STATUS_SUCCESS;
}


enum list_status list_add(struct list *list, void *data) {
    return list_add_at(list, list->size, data);
}


enum list_status list_peek_at(struct list *list, uint32_t index, void **data) {
    if (list == NULL || data == NULL) {
        return LIST_STATUS_INVALID_ARGUMENT;
    }
    if (index >= list->size) {
        return LIST_STATUS_INVALID_INDEX;
    }
    if (list->size == 0) {
        return LIST_STATUS_EMPTY_LIST;
    }

    struct list_node *curr = list->head;
    for (uint32_t i = 0; i < index; i++) {
        curr = curr->next;
    }

    *data = curr->data;
    return LIST_STATUS_SUCCESS;
}


enum list_status list_pop_at(struct list *list, uint32_t index, void **data) {
    if (list == NULL || data == NULL) {
        return LIST_STATUS_INVALID_ARGUMENT;
    }
    if (index >= list->size) {
        return LIST_STATUS_INVALID_INDEX;
    }
    if (list->size == 0) {
        return LIST_STATUS_EMPTY_LIST;
    }

    struct list_node *curr = list->head;
    for (uint32_t i = 0; i < index; i++) {
        curr = curr->next;
    }

    *data = curr->data;

    if (curr->prev != NULL) {
        curr->prev->next = curr->next;
    }
    else {
        list->head = curr->next;
    }

    if (curr->next != NULL) {
        curr->next->prev = curr->prev;
    }

    free(curr);
    list->size--;
    return LIST_STATUS_SUCCESS;
}


enum list_status list_pop_front(struct list *list, void **data) {
    return list_pop_at(list, 0, data);
}
