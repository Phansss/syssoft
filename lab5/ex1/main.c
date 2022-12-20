/**
 * \author Pieter Hanssens
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include"dplist.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

typedef struct {
    int id;
    char* name;
} my_element_t;

dplist_t* list1;
my_element_t* el1;
my_element_t* el2;
my_element_t* el3;



void * element_copy(void* element) {
    my_element_t* copy = malloc(sizeof (my_element_t));
    char* new_name;
    asprintf(&new_name,"%s",((my_element_t*)element)->name);
    assert(copy != NULL);
    copy->id = ((my_element_t*)element)->id;
    copy->name = new_name;
    return (void *) copy;
}

void element_free(void ** element) {
    if (*element == NULL) return;
    else {
        free((((my_element_t*)*element))->name);
        free(*element);
        *element = NULL;
    }
}

int element_compare(void * x, void * y) {
    return ((((my_element_t*)x)->id < ((my_element_t*)y)->id) ? -1 : (((my_element_t*)x)->id == ((my_element_t*)y)->id) ? 0 : 1);
}

/**
 * Callback function to print the contents of an element of type my_element_t inside a dplist.
 * \param element pointer to the element
 * \return Nothing. The content is printed on stdout
*/
void element_print(void* element) {
    printf("(id=%d, name=%s)\n", ((my_element_t*)element)->id, ((my_element_t*)element)->name);
}
/**
* Create a my_element_t element on heap and assign the given id and name.
* \param element new null-pointer, initialized on the caller-stack
* \param id the id of the new element.
* \param name the name of the new element.
* \return 
*/
static my_element_t* element_create(my_element_t* element, int id, char name[]) {
    assert(element==NULL);
    element = malloc(sizeof(my_element_t));
    element->id = id;
    element->name = malloc(sizeof(&name));
    assert(element->name != NULL);
    strcpy(element->name, name);
    return element;
}

int main() {

    list1 = dpl_create(element_copy,
                element_free,
                element_compare,
                element_print);
    el1 = element_create(el1, 1, "el1");
    el2 = element_create(el2, 2, "el2");
    el3 = element_create(el3, 3, "el3");

    dpl_insert_at_index(list1, el3, 0, true);
    dpl_insert_at_index(list1, el2, 0, true);
    dpl_insert_at_index(list1, el1, 0, true);

    dpl_print(list1);

    dpl_free(&list1, true);
    
    return 0;
}

