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

    void (*element_print)(void *element);
};

static bool dpl_is_sorted(dplist_t *list);
dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y),
        void (*element_print)(void *element)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_MEMORY_ERROR);
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    list->element_print = element_print;
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
            //&((*list)->head->element);
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
    //printf("DPL_SIZE: enter\n");
    int count = 0;
    dplist_node_t *dummy;
    if (list == 0) /*{printf("DPL_SIZE: exit\n");*/ return -1;
    if (list->head == NULL) /*{printf("DPL_SIZE: exit\n");*/ return 0;

    for (dummy = list->head; dummy->next != NULL; dummy = dummy->next) {
        count += 1;
         }
    count +=1;
    //printf("DPL_SIZE: exit\n");
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

/** Returns a reference to the first list node of the list.
 * - If the list is empty, NULL is returned.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \return a reference to the first list node of the list or NULL
 */
dplist_node_t *dpl_get_first_reference(dplist_t *list) {
    if (list == NULL) return NULL;
    if (list->head == NULL) return NULL;
    return (list->head);

}

 /** Returns a reference to the last list node of the list.
  * - If the list is empty, NULL is returned.
  * - If 'list' is is NULL, NULL is returned.
  * \param list a pointer to the list
  * \return a reference to the last list node of the list or NULL
  */
 dplist_node_t *dpl_get_last_reference(dplist_t *list) {
    if (list == NULL) return NULL;
    if (list->head == NULL) return NULL;
    dplist_node_t *dummy;
    int count;
    for (dummy = list->head, count=0; dummy->next != NULL; dummy = dummy->next, count++) {
        continue;
    }
    return dummy;
 }

 /** Returns a reference to the next list node of the list node with reference 'reference' in the list.
  * - If the list is empty, NULL is returned.
  * - If 'list' is NULL, NULL is returned.
  * - If 'reference' is NULL, NULL is returned.
  * - If 'reference' is not an existing reference in the list, NULL is returned.
  * \param list a pointer to the list
  * \param reference a pointer to a certain node in the list
  * \return a pointer to the node next to 'reference' in the list or NULL
  */
 dplist_node_t *dpl_get_next_reference(dplist_t *list, dplist_node_t *reference){
    if (list == NULL) return NULL;
    if (list->head == NULL) return NULL;
    if (reference == NULL) return NULL;
    if (dpl_get_index_of_reference(list, reference) == -1) return NULL;
    if (reference->next == NULL) return NULL; //last element in the list
    return (reference->next);
 }

 /** Returns a reference to the previous list node of the list node with reference 'reference' in 'list'.
  * - If the list is empty, NULL is returned.
  * - If 'list' is is NULL, NULL is returned.
  * - If 'reference' is NULL, NULL is returned.
  * - If 'reference' is not an existing reference in the list, NULL is returned.
  * \param list a pointer to the list
  * \param reference a pointer to a certain node in the list
  * \return pointer to the node previous to 'reference' in the list or NULL
  */
 dplist_node_t *dpl_get_previous_reference(dplist_t *list, dplist_node_t *reference) {
    if (list == NULL) return NULL;
    if (list->head == NULL) return NULL;
    if (reference == NULL) return NULL;
    if (dpl_get_index_of_reference(list, reference) == -1) return NULL;
    if (reference->prev == NULL) return NULL; //last element in the list
    return (reference->prev);
 }

 /** Returns a reference to the first list node in the list containing 'element'.
  * - If the list is empty, NULL is returned.
  * - If 'list' is is NULL, NULL is returned.
  * - If 'element' is not found in the list, NULL is returned.
  * \param list a pointer to the list
  * \param element a pointer to an element
  * \return the first list node in the list containing 'element' or NULL
  */
 dplist_node_t *dpl_get_reference_of_element(dplist_t *list, void *element) {
    if (list == NULL) return NULL;
    if (list->head == NULL) return NULL;
    dplist_node_t *dummy = NULL;
    int count;
    for (dummy = list->head, count=0; dummy->next != NULL; dummy = dummy->next, count++) {
       if (dummy->element == element) return dummy;
    }
    if (dummy->element == element) return dummy; //last node check
    return NULL;

 }

/** Returns the index of the list node in the list with reference 'reference'.
 * - the first list node has index 0.
 * - If the list is empty, -1 is returned.
 * - If 'list' is is NULL, -1 is returned.
 * - If 'reference' is NULL, -1 returned.
 * - If 'reference' is not an existing reference in the list, -1 is returned.
 * \param list a pointer to the list
 * \param reference a pointer to a certain node in the list
 * \return the index of the given reference in the list
 */
int dpl_get_index_of_reference(dplist_t *list, dplist_node_t *reference) {
    if (list == NULL) return -1;
    if (list->head == NULL) return -1;
    if (reference == NULL) return -1;
    dplist_node_t *dummy = NULL;
    int count;
    for (dummy = list->head, count=0; dummy->next != NULL; dummy = dummy->next, count++) {
        if (reference == dummy) return count;
    }
    if (reference == dummy) return count;
    return -1;
}

/** Inserts a new list node containing an 'element' in the list at position 'reference'.
 * - If 'list' is NULL, NULL is returned.
 * - If 'reference' is NULL, NULL is returned (nothing is inserted).
 * - If 'reference' is not an existing reference in the list, 'list' is returned (nothing is inserted).
 * \param list a pointer to the list
 * \param element a pointer to an element
 * \param reference a pointer to a certain node in the list
 * \param insert_copy if true use element_copy() to make a copy of 'element' and use the copy in the new list node, otherwise the given element pointer is added to the list
 * \return a pointer to the list or NULL
 */
dplist_t *dpl_insert_at_reference(dplist_t *list, void *element, dplist_node_t *reference, bool insert_copy){
    if (list == NULL) return NULL;
    if (reference == NULL) return NULL; 
    if(list->head == NULL) return list; // When the list is empty, cannot insert at valid reference. -> Reference not found returns list.
    dplist_node_t *dummy = NULL;
    int count;
    for (dummy = list->head, count=0; dummy->next != NULL; dummy = dummy->next, count++) {
        if (reference == dummy) {
            dpl_insert_at_index(list, element, count, insert_copy);
            return list;
        }
    }
    if (reference == dummy) { //Final Node check
        dpl_insert_at_index(list, element, count, insert_copy);
        return list;
    }
    return list;
}


/** Inserts a new list node containing 'element' in the sorted list and returns a pointer to the new list.
 * - The list must be sorted or empty before calling this function.
 * - The sorting is done in ascending order according to a comparison function.
 * - If two members compare as equal, their order in the sorted array is undefined.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param element a pointer to an element
 * \param insert_copy if true use element_copy() to make a copy of 'element' and use the copy in the new list node, otherwise the given element pointer is added to the list
 * \return a pointer to the list or NULL
 */
dplist_t *dpl_insert_sorted(dplist_t *list, void *element, bool insert_copy) {
    if (list == NULL) return NULL;
    if (dpl_is_sorted(list)) {
        if (list->head == NULL) { //empty list
            dpl_insert_at_index(list, element, 0, insert_copy); 
            return list;
        } 
        dplist_node_t* dummy = NULL;
        int count; 
        for (dummy = list->head, count=0; dummy->next != NULL; dummy = dummy->next, count++) {
            if(list->element_compare(element, dummy->element) < 1) {
                dpl_insert_at_index(list, element, count, insert_copy); //if el_dum = 5 and el_dum->next = 6, 
                return list;                                              //then I want el_dum to be inserted AFTER el_dum->next, hence count+2.
            }

        }
        count++;
        dpl_insert_at_index(list, element, count, insert_copy); //insert at end of the list
        return list;
    }
    return NULL;
}

/** Removes the list node with reference 'reference' in the list.
 * - The list node itself should always be freed.
 * - If 'reference' is NULL, NULL is returned (nothing is removed).
 * - If 'reference' is not an existing reference in the list, 'list' is returned (nothing is removed).
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param reference a pointer to a certain node in the list
 * \param free_element if true call element_free() on the element of the list node to remove
 * \return a pointer to the list or NULL
 */
dplist_t *dpl_remove_at_reference(dplist_t *list, dplist_node_t *reference, bool free_element) {
    if (list == NULL) return NULL;
    if (reference == NULL) return NULL; 
    if(list->head == NULL) return list; // When the list is empty, cannot remove at valid reference. -> Reference not found returns list.
    dplist_node_t *dummy = NULL;
    int count;
    for (dummy = list->head, count=0; dummy->next != NULL; dummy = dummy->next, count++) {
        if (reference == dummy) {
            dpl_remove_at_index(list, count, free_element);
            return list;
        }
    }
    if (reference == dummy) { //Final Node check
        count++; //Count not incremented in final for-loop but used in function call immediately below.
        dpl_remove_at_index(list, count, free_element);
        return list;
    }
    return list; 

}

/** Finds the first list node in the list that contains 'element' and removes the list node from 'list'.
 * - If 'element' is not found in 'list', the unmodified 'list' is returned.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param element a pointer to an element
 * \param free_element if true call element_free() on the element of the list node to remove
 * \return a pointer to the list or NULL
 */
dplist_t *dpl_remove_element(dplist_t *list, void *element, bool free_element) {
    if (list == NULL) return NULL;
    if(list->head == NULL) return list;
    dplist_node_t *dummy = NULL;
    int count;
    for (dummy = list->head, count=0; dummy->next != NULL; dummy = dummy->next, count++) {
        if(element == dummy->element) {
            dpl_remove_at_index(list, count, free_element);
            return list;
        }
    }
    if(element == dummy->element) { //Last node check.
        count++; //Count not incremented in final for-loop but used in function call immediately below.
        dpl_remove_at_index(list, count, free_element); 
        return list;
    }
    return list;
}


/** Checks whether the given list is sorted according to the callback function element_compare()
 * - If 'list' is NULL, false is returned.
 * - If the list is empty, the list is sorted.
 * \param list a pointer to the list
 * \return bool indicating whether the list is sorted
*/
static bool dpl_is_sorted(dplist_t *list) {
    if (list == NULL) return false;
    if (list->head == NULL) return true;
    dplist_node_t* dummy = NULL;
    int count; 

    for (dummy = list->head, count=0; dummy->next != NULL; dummy = dummy->next, count++) {
        if ((list->element_compare(dummy->element, dummy->next->element)) > 0) { //element_compare(x,y) returns -1 or 0 if x<=y.
            return false;
        }
    }
    return true;

}

/** Prints the elements in the dplist according to the callback function element_print()
 * - If 'list' is NULL, this is also printed.
*/
void dpl_print(dplist_t *list) {
    if (list == NULL) printf("List is NULL.");
    dplist_node_t *dummy;
    int count;
    printf("%5s dplist contains %d nodes.\n","", dpl_size(list));
    if (dpl_size(list) != 0) {
        printf("%5s INDEX| ELEMENT \n", "");
        printf("%5s------|---------------------------------\n", "");
        for (dummy = list->head, count=0; dummy->next != NULL; dummy = dummy->next, count++) {
            if(count<10) {
                printf("%11d|", count);
            } else if(count <100) {
                printf("%10d|", count);
            } else { 
                printf("%d|", count);
            };
            list->element_print((dummy->element));
        }
        if(count<10) {
                printf("%11d|", count);
            } else if(count <100) {
                printf("%10d|", count);
            } else { 
                printf("%d|", count);
            };
            list->element_print((dummy->element));
        
        //printf("----------------------------------------\n\n");
        printf("\n");
    }
    else {
    }
}









