/**
 * A generic doubly-linked list to store data in the assembler and simulator.
 *
 * @author Jonathan Uhler
 */


#ifndef _STRUCTURES_LIST_H_
#define _STRUCTURES_LIST_H_


#include <stdint.h>


/**
 * A single data node in the list.
 */
struct list_node {
    /** Pointer to the data held in the node. The user is responsible for casting to another type */
    void *data;
    /** Pointer to the next node in the list (higher index). */
    struct list_node *next;
    /** Pointer to the previous node in the list (lower index). */
    struct list_node *prev;
};


/**
 * A list of zero or more nodes.
 */
struct list {
    /** Pointer to the head node of the list. */
    struct list_node *head;
    /** Number of nodes in the list. */
    uint32_t size;
};


/**
 * Status of list API calls.
 */
enum list_status {
    /** The list API function completed successfully. */
    LIST_STATUS_SUCCESS = 0,
    /** The list API function did not complete because the list is empty. */
    LIST_STATUS_EMPTY_LIST,
    /** The list API function did not complete because the provided index is invalid. */
    LIST_STATUS_INVALID_INDEX,
    /** The list API function did not complete because it was called incorrectly. */
    LIST_STATUS_INVALID_ARGUMENT
};


/** Type definition for a user-defined function called to free data while destructing a list. */
typedef void (*list_node_free_callback)(void *data);


/**
 * Creates a new list with zero elements.
 *
 * @return Pointer to the created list.
 */
struct list *create_list();


/**
 * Destructs a list created with create_list.
 *
 * @param list                The list to destroy.
 * @param node_free_callback  An optional function pointer that will be called on each node's
 *                            void *data pointer to free it (if dynamically allocated). If the node
 *                            data should not be freed, NULL should be passed.
 */
void destroy_list(struct list *list, list_node_free_callback node_free_callback);


/**
 * Frees a node's void *data pointer by calling free(data) if data is not NULL.
 *
 * This function can be passed as the node_free_callback parameter in destroy_list for simple
 * dynamically-allocated node data.
 *
 * @param data  The data to free.
 */
void list_default_node_free_callback(void *data);


/**
 * Add the provided data to the list at the specified index.
 *
 * @param list   The list to add data to.
 * @param index  The index to insert the data at. The new node will be inserted such that it
 *               occupies the provided index. Indices strictly larger than the size of the list
 *               are invalid.
 * @param data   The data to add.
 *
 * @return The status of the add operation.
 */
enum list_status list_add_at(struct list *list, uint32_t index, void *data);


/**
 * Add the provided data to the end of the list.
 *
 * This function is equivalent to list_add_at(list, list->size, data).
 *
 * @param list  The list to add data to.
 * @param data  The data to add.
 *
 * @return The status of the add operation.
 */
enum list_status list_add(struct list *list, void *data);


/**
 * Gets the data from the specified index in the list.
 *
 * @param list   The list to get data from.
 * @param index  The index to get the data at.
 * @param data   A pointer to place the data.
 *
 * @return The status of the peek operation. If not successful, no guarantee is made about the
 *         void * pointer stored in data.
 */
enum list_status list_peek_at(struct list *list, uint32_t index, void **data);


/**
 * Gets and removes the data from the specified index in the list.
 *
 * @param list   The list to get data from.
 * @param index  The index to get the data at.
 * @param data   A pointer to place the data.
 *
 * @return The status of the pop operation. If not successful, no guarantee is made about the
 *         void * pointer stored in data, but it is guaranteed that the data node will not be
 *         removed from the list.
 */
enum list_status list_pop_at(struct list *list, uint32_t index, void **data);


/**
 * Gets and removes the data from the first node in the list.
 *
 * This function is equivalent to list_pop_at(list, 0, data).
 *
 * @param list  The list to get data from.
 * @param data  A pointer to place the data.
 *
 * @return The status of the pop operation. If not successful, no guarantee is made about the
 *         void * pointer stored in data, but it is guaranteed that the data node will not be
 *         removed from the list.
 */
enum list_status list_pop_front(struct list *list, void **data);


#endif  // _STRUCTURES_LIST_H_
