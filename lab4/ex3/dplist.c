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


dplist_t *dpl_create(
        void* (*element_copy)(void *element),
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
            (*list)->element_free(&((*list)->head->element));
        }
        free((*list)->head);
        free(*list);
        *list = NULL;
        return;
    }
    else {
        curr_node = (*list)->head->next;    //Start at second node.
        if (free_element) {
            (*list)->element_free(&(curr_node->prev->element));
        }
        free(curr_node->prev);              //Free the first node.
        
        while(curr_node->next != NULL){
            curr_node = curr_node->next;
            if (free_element) {
                (*list)->element_free(&(curr_node->prev->element));
            }
            free(curr_node->prev);
        }
        if (free_element) {
            (*list)->element_free(&(curr_node->element));
        }
        free(curr_node);
        free(*list);
        *list = NULL;
        return;
    }

}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {

    dplist_node_t *ref_at_index, *list_node;
    if (list == NULL) {
        return NULL;
        }
    list_node = malloc(sizeof(dplist_node_t));
    DPLIST_ERR_HANDLER(list_node == NULL, DPLIST_MEMORY_ERROR);
    if (insert_copy) {
        list_node->element = list->element_copy(element);
    }
    else {
        list_node->element = element;
    }
    //printf(element);
    // pointer drawing breakpoint
    if (list->head == NULL) { // covers case 1
        list_node->prev = NULL;
        list_node->next = NULL;
        list->head = list_node;
        // pointer drawing breakpoint
    } else if (index <= 0) { // covers case 2
        list_node->prev = NULL;
        list_node->next = list->head;
        list->head->prev = list_node;
        list->head = list_node;
        // pointer drawing breakpoint
    } else {
        ref_at_index = dpl_get_reference_at_index(list, index);
        assert(ref_at_index != NULL);
        // pointer drawing breakpoint
        if (index < dpl_size(list)) { // covers case 4
            list_node->prev = ref_at_index->prev;
            list_node->next = ref_at_index;
            ref_at_index->prev->next = list_node;
            ref_at_index->prev = list_node;
            // pointer drawing breakpoint
        } else { // covers case 3 (case 3: insert at end of the list)
            assert(ref_at_index->next == NULL);
            list_node->next = NULL;
            list_node->prev = ref_at_index;
            ref_at_index->next = list_node;
            // pointer drawing breakpoint
        }
    }
    return list;

}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {
    if (list == NULL) return NULL;
    
    dplist_node_t *temp = NULL;


    if (list->head == NULL) { // covers case 1: empty list!
        return list;
        // pointer drawing breakpoint

    } else if (index <= 0) { // covers case 2: do operation at start of the list
        temp = list->head; //temporary pointer to node 0

        if (list->head->next != NULL) { //if-statement necessary for when only one element in the list.
            list->head->next->prev = NULL; //make current node 1, node 0 (prev == NULL)
            list->head = list->head->next; //create link between dplist and node 
        }
        else {
            list->head = NULL;
        }

        if (free_element) {
            list->element_free(&(temp->element));
        }
        free(temp);
        // pointer drawing breakpoint
    } else {
        temp = dpl_get_reference_at_index(list, index);
        assert(temp != NULL);
        // pointer drawing breakpoint


        if (index < dpl_size(list)) { // covers case 4: do operation in the middle of the list
            temp->prev->next = temp->next;
            temp->next->prev = temp->prev;
            if (free_element) {
                list->element_free(&(temp->element));
            }
            free(temp);
            // pointer drawing breakpoint
        } else { // covers case 3: do operation at the end of the list
            assert(temp->next == NULL);
            if (temp->prev == NULL) {
                list->head = NULL;
            } 
            else {
                temp->prev->next = NULL;
            }
            if (free_element) {
                list->element_free(&(temp->element));
            }
            free(temp);
            temp = NULL;
            // pointer drawing breakpoint
        }
    }
    return list;
}

int dpl_size(dplist_t *list) {

    int count = 0;
    dplist_node_t *dummy;
    if (list == 0) return -1;
    if (list->head == NULL) return 0;

    for (dummy = list->head; dummy->next != NULL; dummy = dummy->next) {
        count += 1;
         }
    count +=1;
    return count;

}

void *dpl_get_element_at_index(dplist_t *list, int index) {
    if (list == NULL) return 0;
    if (list->head == NULL) return 0;
    int count;
    dplist_node_t *dummy;

    if (index <= 0) return (list->head->element);

    for (dummy = list->head, count = 0; dummy->next != NULL; dummy = dummy->next, count++) {
        if (count >= index) return (dummy->element);
    }
    if (count >= index) return (dummy->element); //final node check
    return (dummy->element);

}

int dpl_get_index_of_element(dplist_t *list, void *element) {
    if (list == NULL) return -1;
    if (list->head == NULL) return -1;
    int count;
    dplist_node_t *dummy;
    for (dummy = list->head, count = 0; dummy->next != NULL; dummy = dummy->next, count++) {
        if (list->element_compare(dummy->element, element) == 0) return count;
    }
    if (list->element_compare(dummy->element, element) == 0) return count; //final node check
    return -1;

}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {

    int count;
    dplist_node_t *dummy;

    if (list == NULL) return NULL;
    if (list->head == NULL) return NULL;

    for (dummy = list->head, count = 0; dummy->next != NULL; dummy = dummy->next, count++) {
        if (count >= index) return dummy;
    }
    if (count >= index) return dummy; //final node check
    return dummy;
}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {
    if (list == NULL) return NULL;
    if (list->head == NULL) return NULL;
    if (reference == NULL) return NULL;
    int count;
    dplist_node_t *dummy;
    dummy = NULL;
    for (dummy = list->head, count = 0; dummy->next != NULL; dummy = dummy->next, count++) {
        if (dummy == reference) {
            return dummy->element;
        }
    }
    if (dummy == reference) { //final node check
        return dummy->element; 
        }
    return NULL;
    
}

/* void dpl_print(dplist_t *list) {
    dplist_node_t *dummy;
    int count;
    printf("\nList contains %d nodes.\n", dpl_size(list));
    if (dpl_size(list) != 0) {
        for (dummy = list->head, count=0; dummy->next != NULL; dummy = dummy->next, count++) {
            list->element_print((dummy->element));
        }
        list->element_print((dummy->element));
    } */
    
//}




