/**
 * \author Pieter Hanssens
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * definition of error codes
 * */
#define DPLIST_NO_ERROR 0
#define DPLIST_MEMORY_ERROR 1 // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list
 

#ifdef DEBUG
#define DEBUG_PRINTF(...) 									                                        \
        do {											                                            \
            fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	    \
            fprintf(stderr,__VA_ARGS__);								                            \
            fflush(stderr);                                                                         \
                } while(0)
#else
#define DEBUG_PRINTF(...) (void)0
#endif


#define DPLIST_ERR_HANDLER(condition, err_code)                         \
    do {                                                                \
            if ((condition)) DEBUG_PRINTF(#condition " failed\n");      \
            assert(!(condition));                                       \
        } while(0)


/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};


dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_MEMORY_ERROR);
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}


void dpl_free(dplist_t **list, bool free_element) {
    if ((*list) == NULL) return;
    dplist_node_t *curr_node = NULL;

    if ((*list)->head == NULL){ //No nodes in list.
        free(*list);
        *list = NULL;
        return;
    }
    else if ((*list)->head->next == NULL) { //Only one node in the list.
        if (free_element) {
            (*list)->element_free(((*list)->head->element));
        }
        free((*list)->head);
        free(*list);
        *list = NULL;
        return;
    }
    else {
        curr_node = (*list)->head->next;    //Start at second node.
        if (free_element) {
            (*list)->element_free(curr_node->prev->element);
        }
        free(curr_node->prev);              //Free the first node.
        
        while(curr_node->next != NULL){
            curr_node = curr_node->next;
            if (free_element) {
                (*list)->element_free(curr_node->prev->element);
            }
            free(curr_node->prev);
        }
        if (free_element) {
            (*list)->element_free(curr_node->element);
        }
        free(curr_node);
        free(*list);
        *list = NULL;
        return;
    }

}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {

    //TODO: add your code here

}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {

    //TODO: add your code here

}

int dpl_size(dplist_t *list) {

    //TODO: add your code here

}

void *dpl_get_element_at_index(dplist_t *list, int index) {

    //TODO: add your code here

}

int dpl_get_index_of_element(dplist_t *list, void *element) {

    //TODO: add your code here

}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {

    //TODO: add your code here

}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {

    //TODO: add your code here

}


